#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_INT(num) \
    ConstantInt::get(num, module.get())


// You can define global variables here
// to store state
Value* global_v;
Value* global_p;
size_t name_count;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

void CminusfBuilder::visit(ASTProgram &node) {
    for (auto &decl : node.declarations) {
        decl->accept(*this);
    }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    std::vector<Type*> param_t;
    for (auto &p : node.params) {
        if (p->type == TYPE_INT) {
            if (p->isarray) {
                param_t.push_back(Type::get_int32_ptr_type(module.get()));
            } else {
                param_t.push_back(Type::get_int32_type(module.get()));
            }
        } else if (p->type == TYPE_FLOAT) {
            if (p->isarray) {
                param_t.push_back(Type::get_float_ptr_type(module.get()));
            } else {
                param_t.push_back(Type::get_float_type(module.get()));
            }
        }
    }

    FunctionType* func_t;
    if (node.type == TYPE_INT) {
        func_t = FunctionType::get(Type::get_int32_type(module.get()), param_t);
    } else if (node.type == TYPE_FLOAT) {
        func_t = FunctionType::get(Type::get_float_type(module.get()), param_t);
    } else if (node.type == TYPE_VOID) {
        func_t = FunctionType::get(Type::get_void_type(module.get()), param_t);
    }
    auto func = Function::create(func_t, node.id, module.get());
    scope.push(node.id, func);
    auto entryBB = BasicBlock::create(module.get(), "entry", func);
    builder->set_insert_point(entryBB);

    scope.enter();
    auto p = node.params.begin();
    for (auto arg = func->arg_begin(); arg != func->arg_end(); arg++) {
        (*(p++))->accept(*this);
        builder->create_store(*arg, global_p);
    }
    node.compound_stmt->accept(*this);
    if (builder->get_insert_block()->get_num_of_instr() == 0) {
        if (node.type == TYPE_VOID) {
            builder->create_void_ret();
        } else if (node.type == TYPE_INT){
            builder->create_ret(CONST_INT(0));
        } else if (node.type == TYPE_FLOAT) {
            builder->create_ret(CONST_FP(0));
        } else {
            std::abort();
        }
//        builder->get_insert_block()->erase_from_parent();
    }

    scope.exit();
}

void CminusfBuilder::visit(ASTVarDeclaration &node) {
    if (node.num != nullptr) {
        // array
        if (node.type == TYPE_INT) {
            auto int_t = Type::get_int32_type(module.get());
            auto array_t = ArrayType::get_array_type(int_t, node.num->i_val);
            if (scope.in_global()){
                auto initializer = ConstantZero::get(array_t, module.get());
                global_v = GlobalVariable::create(node.id, module.get(), array_t, false, initializer);
            } else {
                global_v = builder->create_alloca(array_t);
            }
        } else if (node.type == TYPE_FLOAT) {
            auto float_t = Type::get_float_type(module.get());
            auto array_t = ArrayType::get_array_type(float_t, node.num->i_val);
            if (scope.in_global()) {
                auto initializer = ConstantZero::get(array_t, module.get());
                global_v = GlobalVariable::create(node.id, module.get(), array_t, false, initializer);
            } else {
                global_v = builder->create_alloca(array_t);
            }
        }
    } else {
        // variable
        if (node.type == TYPE_INT) {
            auto int_t = Type::get_int32_type(module.get());
            if (scope.in_global()) {
                auto initializer = ConstantZero::get(int_t, module.get());
                global_v = GlobalVariable::create(node.id, module.get(), int_t, false, initializer);
            } else {
                global_v = builder->create_alloca(int_t);
            }
        } else if (node.type == TYPE_FLOAT) {
            auto float_t = Type::get_float_type(module.get());
            if (builder->get_insert_block() == nullptr) {
                auto initializer = ConstantZero::get(float_t, module.get());
                global_v = GlobalVariable::create(node.id, module.get(), float_t, false, initializer);
            } else {
                global_v = builder->create_alloca(float_t);
            }
        }
    }

    scope.push(node.id, global_v);
}

void CminusfBuilder::visit(ASTParam &node) {
    if (node.isarray) {
        if (node.type == TYPE_INT) {
            auto arr_t = Type::get_int32_ptr_type(module.get());
            global_p = builder->create_alloca(arr_t);
        } else if (node.type == TYPE_FLOAT) {
            auto arr_t = Type::get_float_ptr_type(module.get());
            global_p = builder->create_alloca(arr_t);
        }
    } else {
        if (node.type == TYPE_INT) {
            auto int_t = Type::get_int32_type(module.get());
            global_p = builder->create_alloca(int_t);
        } else if (node.type == TYPE_FLOAT) {
            auto float_t = Type::get_float_type(module.get());
            global_p = builder->create_alloca(float_t);
        }
    }

    scope.push(node.id, global_p);
}

void CminusfBuilder::visit(ASTCompoundStmt &node) {
    scope.enter();
    for (auto &d : node.local_declarations) {
        d->accept(*this);
    }
    for (auto &s : node.statement_list) {
        s->accept(*this);
    }
    scope.exit();
}

void CminusfBuilder::visit(ASTNum &node) {
    if (node.type == TYPE_FLOAT) {
        global_v = CONST_FP(node.f_val);
    } else if (node.type == TYPE_INT) {
        global_v = ConstantInt::get(node.i_val, module.get());
    }
}

void CminusfBuilder::visit(ASTExpressionStmt &node) {
    if (node.expression != nullptr) {
        node.expression->accept(*this);
    }
}

void CminusfBuilder::visit(ASTSelectionStmt &node) {
    auto parent_func = builder->get_insert_block()->get_parent();
    auto trueBB = BasicBlock::create(module.get(), "TrueBB" + std::to_string(name_count++), parent_func);
    auto falseBB = BasicBlock::create(module.get(), "FalseBB" + std::to_string(name_count++), parent_func);

    node.expression->accept(*this);
    auto cmp = global_v;
    if (cmp->get_type()->is_float_type()) {
        cmp = builder->create_fcmp_ne(cmp, CONST_FP(0));
    } else if (cmp->get_type()->is_integer_type()) {
        auto *type = static_cast<IntegerType*>(cmp->get_type());
        if (type->get_num_bits() == 32) {
            cmp = builder->create_icmp_ne(cmp, CONST_INT(0));
        }
    }

    builder->create_cond_br(cmp, trueBB, falseBB);

//    if(builder->get_insert_block()->get_terminator() == nullptr) {
//        builder->create_br(trueBB);
//    }
    builder->set_insert_point(trueBB);
    node.if_statement->accept(*this);

    if (node.else_statement != nullptr) {
        auto retBB = BasicBlock::create(module.get(), "ReturnBB" + std::to_string(name_count++), parent_func);
        builder->create_br(retBB);
        builder->set_insert_point(falseBB);
        node.else_statement->accept(*this);
        if (builder->get_insert_block()->get_terminator() == nullptr) {
            builder->create_br(retBB);
        }
        builder->set_insert_point(retBB);
    } else {
        if (builder->get_insert_block()->get_terminator() == nullptr) {
            builder->create_br(falseBB);
        }
        builder->set_insert_point(falseBB);
    }

}

void CminusfBuilder::visit(ASTIterationStmt &node) {
    auto parent_func = builder->get_insert_block()->get_parent();
    auto conBB = BasicBlock::create(module.get(), "ConditionBB" + std::to_string(name_count++), parent_func);
    auto iterBB = BasicBlock::create(module.get(), "IterationBB" + std::to_string(name_count++), parent_func);
    auto retBB = BasicBlock::create(module.get(), "ReturnBB" + std::to_string(name_count++), parent_func);

    builder->create_br(conBB);
    builder->set_insert_point(conBB);
    node.expression->accept(*this);
    auto cmp = global_v;
    if (cmp->get_type()->is_float_type()) {
        cmp = builder->create_fcmp_ne(cmp, CONST_FP(0));
    } else if (cmp->get_type()->is_integer_type()) {
        auto *type = static_cast<IntegerType*>(cmp->get_type());
        if (type->get_num_bits() == 32) {
            cmp = builder->create_icmp_ne(cmp, CONST_INT(0));
        }
    }

    builder->create_cond_br(cmp, iterBB, retBB);

    builder->set_insert_point(iterBB);
    node.statement->accept(*this);
    if (builder->get_insert_block()->get_terminator() == nullptr) {
        builder->create_br(conBB);
    }

    builder->set_insert_point(retBB);
}

void CminusfBuilder::visit(ASTReturnStmt &node) {
    auto current_f = builder->get_insert_block()->get_parent();
    if (node.expression == nullptr) {
        // check whether the function is really void type
        if (!current_f->get_return_type()->is_void_type()) {
            std::cout << "Not right! Current function should not return void type" << std::endl;
            std::abort();
        } else {
            builder->create_void_ret();
        }
    } else {
        node.expression->accept(*this);
        if (global_v->get_type()->is_integer_type()) {
            auto *type = static_cast<IntegerType*>(global_v->get_type());
            if (type->get_num_bits() == 1) {
                global_v = builder->create_zext(global_v, Type::get_int32_type(module.get()));
            }
        }
        if (current_f->get_return_type()->is_integer_type() and global_v->get_type()->is_float_type()) {
            global_v = builder->create_fptosi(global_v, Type::get_int32_type(module.get()));
        }
        builder->create_ret(global_v);
    }
}

void CminusfBuilder::visit(ASTAssignExpression &node) {
    node.var->accept(*this);
    auto p = global_p;
    auto v = global_v;
    // Note that p must be pointer, not loaded value
    node.expression->accept(*this);
    if (global_v->get_type()->is_integer_type()) {
        auto *type = static_cast<IntegerType*>(global_v->get_type());
        if (type->get_num_bits() == 1) {
            global_v = builder->create_zext(global_v, Type::get_int32_type(module.get()));
        }
    }
    if (v->get_type()->is_integer_type() and global_v->get_type()->is_float_type()) {
        global_v = builder->create_fptosi(global_v, Type::get_int32_type(module.get()));
    } else if (v->get_type()->is_float_type() and global_v->get_type()->is_integer_type()) {
        global_v = builder->create_sitofp(global_v, Type::get_float_type(module.get()));
    }
    builder->create_store(global_v, p);
}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    node.additive_expression_l->accept(*this);
    if (node.additive_expression_r != nullptr) {
        auto lLoad = global_v;
        node.additive_expression_r->accept(*this);
        auto rLoad = global_v;

        if (lLoad->get_type()->is_integer_type() and rLoad->get_type()->is_integer_type()) {
            if (node.op == OP_LT) {
                global_v = builder->create_icmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                global_v = builder->create_icmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                global_v = builder->create_icmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                global_v = builder->create_icmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                global_v = builder->create_icmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                global_v = builder->create_icmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        } else if (lLoad->get_type()->is_float_type() and rLoad->get_type()->is_integer_type()) {
            auto float_t = Type::get_float_type(module.get());
            rLoad = builder->create_sitofp(rLoad, float_t);
            if (node.op == OP_LT) {
                global_v = builder->create_fcmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                global_v = builder->create_fcmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                global_v = builder->create_fcmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                global_v = builder->create_fcmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                global_v = builder->create_fcmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                global_v = builder->create_fcmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        } else if (rLoad->get_type()->is_float_type() and lLoad->get_type()->is_integer_type()) {
            auto float_t = Type::get_float_type(module.get());
            lLoad = builder->create_sitofp(lLoad, float_t);
            if (node.op == OP_LT) {
                global_v = builder->create_fcmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                global_v = builder->create_fcmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                global_v = builder->create_fcmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                global_v = builder->create_fcmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                global_v = builder->create_fcmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                global_v = builder->create_fcmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        } else {
            if (node.op == OP_LT) {
                global_v = builder->create_fcmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                global_v = builder->create_fcmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                global_v = builder->create_fcmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                global_v = builder->create_fcmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                global_v = builder->create_fcmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                global_v = builder->create_fcmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        }
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    node.term->accept(*this);
    if (node.additive_expression != nullptr) {
        auto rLoad = global_v;
        node.additive_expression->accept(*this);
        auto lLoad = global_v;
        if (lLoad->get_type()->is_integer_type()) {
            auto lType = static_cast<IntegerType*>(lLoad->get_type());
            if (lType->get_num_bits() == 1) {
                lLoad = builder->create_zext(lLoad, Type::get_int32_type(module.get()));
            }
        }
        if (rLoad->get_type()->is_integer_type()) {
            auto rType = static_cast<IntegerType*>(rLoad->get_type());
            if (rType->get_num_bits() == 1) {
                rLoad = builder->create_zext(rLoad, Type::get_int32_type(module.get()));
            }
        }
        if (lLoad->get_type()->is_integer_type() and rLoad->get_type()->is_integer_type()) {
            switch (node.op) {
                case OP_PLUS:
                    global_v = builder->create_iadd(lLoad, rLoad);
                    break;
                case OP_MINUS:
                    global_v = builder->create_isub(lLoad, rLoad);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (lLoad->get_type()->is_integer_type() and rLoad->get_type()->is_float_type()) {
            auto float_t = Type::get_float_type(module.get());
            lLoad = builder->create_sitofp(lLoad, float_t);
            switch (node.op) {
                case OP_PLUS:
                    global_v = builder->create_fadd(lLoad, rLoad);
                    break;
                case OP_MINUS:
                    global_v = builder->create_fsub(lLoad, rLoad);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (rLoad->get_type()->is_integer_type() and lLoad->get_type()->is_float_type()) {
            auto float_t = Type::get_float_type(module.get());
            rLoad = builder->create_sitofp(rLoad, float_t);
            switch (node.op) {
                case OP_PLUS:
                    global_v = builder->create_fadd(lLoad, rLoad);
                    break;
                case OP_MINUS:
                    global_v = builder->create_fsub(lLoad, rLoad);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else {
            switch (node.op) {
                case OP_PLUS:
                    global_v = builder->create_fadd(lLoad, rLoad);
                    break;
                case OP_MINUS:
                    global_v = builder->create_fsub(lLoad, rLoad);
                    break;
                default:
                    // err
                    std::abort();
            }
        }
    }
}

void CminusfBuilder::visit(ASTTerm &node) {
    node.factor->accept(*this);
    if (node.term != nullptr) {
        auto r = global_v;
        node.term->accept(*this);
        auto l = global_v;
        if (l->get_type()->is_float_type() and r->get_type()->is_float_type()) {
            switch (node.op) {
                case OP_MUL:
                    global_v = builder->create_fmul(l, r);
                    break;
                case OP_DIV:
                    global_v = builder->create_fdiv(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (l->get_type()->is_integer_type() and r->get_type()->is_float_type()) {
            auto float_t = Type::get_float_type(module.get());
            l = builder->create_sitofp(l, float_t);
            switch (node.op) {
                case OP_MUL:
                    global_v = builder->create_fmul(l, r);
                    break;
                case OP_DIV:
                    global_v = builder->create_fdiv(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (r->get_type()->is_integer_type() and l->get_type()->is_float_type()) {
            auto float_t = Type::get_float_type(module.get());
            r = builder->create_sitofp(r, float_t);
            switch (node.op) {
                case OP_MUL:
                    global_v = builder->create_fmul(l, r);
                    break;
                case OP_DIV:
                    global_v = builder->create_fdiv(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else {
            switch (node.op) {
                case OP_MUL:
                    global_v = builder->create_imul(l, r);
                    break;
                case OP_DIV:
                    global_v = builder->create_isdiv(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        }
    }
}

void CminusfBuilder::visit(ASTVar &node) {
    if (node.expression != nullptr) {
        auto parent_func = builder->get_insert_block()->get_parent();
        auto TrueBB = BasicBlock::create(module.get(), "NegativeIDXBB" + std::to_string(name_count++), parent_func);
        auto FalseBB = BasicBlock::create(module.get(), "RightIDXBB" + std::to_string(name_count++), parent_func);

        node.expression->accept(*this);
        auto index = global_v;
        if (index->get_type()->is_float_type()) {
            index = builder->create_fptosi(index, Type::get_int32_type(module.get()));
        }

        auto cmp = builder->create_icmp_lt(index, CONST_INT(0));
        builder->create_cond_br(cmp, TrueBB, FalseBB);
        builder->set_insert_point(TrueBB);
        builder->create_call(scope.find("neg_idx_except"), {});
        builder->create_br(FalseBB);
        builder->set_insert_point(FalseBB);

        auto alloca = scope.find(node.id);
        if (alloca->get_type()->get_pointer_element_type()->is_array_type()) {
            global_p = builder->create_gep(alloca, {CONST_INT(0), index});
        } else {
            auto pLoad = builder->create_load(alloca);
            global_p = builder->create_gep(pLoad, {index});
        }
        global_v = builder->create_load(global_p);
    } else {
        global_p = scope.find(node.id);
        global_v = builder->create_load(global_p);
    }
}

void CminusfBuilder::visit(ASTCall &node) {
    auto f = dynamic_cast<Function*>(scope.find(node.id));
    std::vector<Value*> args;
    auto param = f->arg_begin();
    for (auto &a : node.args) {
        a->accept(*this);
        if (global_v->get_type()->is_integer_type()) {
            auto type = static_cast<IntegerType*>(global_v->get_type());
            if (type->get_num_bits() == 1) {
                global_v = builder->create_zext(global_v, Type::get_int32_type(module.get()));
            }
        }
        if ((*param)->get_type()->is_float_type() and global_v->get_type()->is_integer_type()) {
            global_v = builder->create_sitofp(global_v, Type::get_float_type(module.get()));
        } else if ((*param)->get_type()->is_integer_type() and global_v->get_type()->is_float_type()) {
            global_v = builder->create_fptosi(global_v, Type::get_int32_type(module.get()));
        } else if (global_v->get_type()->is_array_type()) {
            global_v = builder->create_gep(global_p, {CONST_INT(0), CONST_INT(0)});
        }
        param++;
        args.push_back(global_v);
    }

    global_v = builder->create_call(f, args);
}
