#include "cminusf_builder.hpp"

// use these macros to get constant value
#define CONST_FP(num) \
    ConstantFP::get((float)num, module.get())
#define CONST_ZERO(type) \
    ConstantZero::get(var_type, module.get())


// You can define global variables here
// to store state

std::shared_ptr<Value> global_v;
std::vector<BasicBlock*> bb_stack;
size_t name_count;

/*
 * use CMinusfBuilder::Scope to construct scopes
 * scope.enter: enter a new scope
 * scope.exit: exit current scope
 * scope.push: add a new binding to current scope
 * scope.find: find and return the value bound to the name
 */

// TODO: add INT1 support

void CminusfBuilder::visit(ASTProgram &node) {
    for (auto &decl : node.declarations) {
        decl->accept(*this);
    }
    // 下面的代码是错误尝试，报告中详细解释。
    // for (auto &decl : node.declarations) {
    //     auto var_decl = dynamic_cast<ASTVarDeclaration*>(&node);
    //     if (var_decl) {
    //         // var_decl->accept(*this);
    //         if (var_decl->num == nullptr) {
    //             // 声明变量
    //             Value* var_alloca;
    //             if (var_decl->type == TYPE_INT) {
    //                 auto int_t = Type::get_int32_type(module.get());
    //                 auto initializer = ConstantZero::get(int_t, module.get());
    //                 var_alloca = GlobalVariable::create("global_v" + std::to_string(name_count++), module.get(), int_t, false, initializer);
    //                 // var_alloca = builder->create_alloca(int_t);
    //             } else if (var_decl->type == TYPE_FLOAT) {
    //                 auto float_t = Type::get_float_type(module.get());
    //                 auto initializer = ConstantZero::get(float_t, module.get());
    //                 var_alloca = GlobalVariable::create("global_v" + std::to_string(name_count++), module.get(), float_t, false, initializer);
    //             }
    //             scope.push(var_decl->id, var_alloca);
    //         } else {
    //             // 声明数组
    //             Value* arr_alloca;
    //             if (var_decl->type == TYPE_INT) {
    //                 auto int_t = Type::get_int32_type(module.get());
    //                 if (var_decl->num->i_val < 0) {
    //                     builder->create_call(scope.find("neg_idx_except_fun"), {});
    //                 }
    //                 auto arr_t = Type::get_array_type(int_t, var_decl->num->i_val);
    //                 auto initializer = ConstantZero::get(arr_t, module.get());
    //                 arr_alloca = GlobalVariable::create("global_arr" + std::to_string(name_count++), module.get(), arr_t, false, initializer);
    //                 // arr_alloca = builder->create_alloca(arr_t);
    //             } else if (var_decl->type == TYPE_FLOAT) {
    //                 auto float_t = Type::get_float_type(module.get());
    //                 auto arr_t = Type::get_array_type(float_t, var_decl->num->i_val);
    //                 auto initializer = ConstantZero::get(arr_t, module.get());
    //                 arr_alloca = GlobalVariable::create("global_arr" + std::to_string(name_count++), module.get(), arr_t, false, initializer);
    //                 // arr_alloca = builder->create_alloca(arr_t);
    //             }
    //             scope.push(var_decl->id, arr_alloca);
    //         }
    //         continue;
    //     }

    //     auto fun_decl = dynamic_cast<ASTFunDeclaration*>(&node);
    //     if (fun_decl) {
    //         fun_decl->accept(*this);
    //         continue;
    //     }
    // }
}

void CminusfBuilder::visit(ASTFunDeclaration &node) {
    scope.enter();
    std::vector<Value*> param_l;
    std::vector<Type*> param_t;
    for (auto &p : node.params) {
        p->accept(*this);
        param_l.push_back(global_v.get());
        if (p->type == TYPE_INT) {
            param_t.push_back(Type::get_int32_type(module.get()));
        } else if (p->type == TYPE_FLOAT) {
            param_t.push_back(Type::get_float_type(module.get()));
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
    auto entryBB = BasicBlock::create(module.get(), "entry", func);
    builder->set_insert_point(entryBB);
    
    node.compound_stmt->accept(*this);
    
    scope.exit();
    scope.push(node.id, func);
}


void CminusfBuilder::visit(ASTParam &node) {
    if (node.isarray) {
        if (node.type == TYPE_INT) {
            auto arr_t = Type::get_int32_ptr_type(module.get());
            global_v = std::make_shared<Value>(builder->create_alloca(arr_t));
        } else if (node.type == TYPE_FLOAT) {
            auto arr_t = Type::get_float_ptr_type(module.get());
            global_v = std::make_shared<Value>(builder->create_alloca(arr_t));
        }
    } else {
        if (node.type == TYPE_INT) {
            auto int_t = Type::get_int32_type(module.get());
            global_v = std::make_shared<Value>(builder->create_alloca(int_t));
        } else if (node.type == TYPE_FLOAT) {
            auto float_t = Type::get_float_type(module.get());
            global_v = std::make_shared<Value>(builder->create_alloca(float_t));
        }
    }

    scope.push(node.id, global_v.get());
}


void CminusfBuilder::visit(ASTNum &node) {
    if (node.type == TYPE_FLOAT) {
        global_v = std::make_shared<Value>(CONST_FP(node.f_val));
    } else if (node.type == TYPE_INT) {
        global_v = std::make_shared<Value>(ConstantInt::get(node.i_val, module.get()));
    }
}

void CminusfBuilder::visit(ASTVarDeclaration &node) { }

}


void CminusfBuilder::visit(ASTCompoundStmt &node) { }

void CminusfBuilder::visit(ASTExpressionStmt &node) { }

void CminusfBuilder::visit(ASTSelectionStmt &node) { }

void CminusfBuilder::visit(ASTIterationStmt &node) { }

void CminusfBuilder::visit(ASTReturnStmt &node) {
    if (node.expression == nullptr) {
        // 检查是否函数本身真的是void型
        auto current_f = builder->get_insert_block()->get_parent();
        if (!current_f->get_return_type()->is_void_type()) {
            std::cout << "Not right! Current function should not return void type" << std::endl;
            std::abort();
        } else {
            builder->create_ret(nullptr);
        }
    } else {
        node.expression->accept(*this);
        // auto assign_expr = dynamic_cast<ASTAssignExpression*>(&node.expression);
        // if (assign_expr) {
        //     assign_expr->accept(*this);
        // }
        // auto simple_expr = dynamic_cast<ASTSimpleExpression*>(&node.expression);
        // if (simple_expr) {
        //     simple_expr->accept(*this);
        // }
        // builder->create_ret(global_v.get());
    }
}

void CminusfBuilder::visit(ASTVar &node) {
    if (node.expression != nullptr) {
        auto v = scope.find(node.id);
        node.expression->accept(*this);
        builder->create_load(v);
    }
}

void CminusfBuilder::visit(ASTAssignExpression &node) {

}

void CminusfBuilder::visit(ASTSimpleExpression &node) {
    if (node.additive_expression_r == nullptr) {
        node.additive_expression_l->accept(*this);
    } else {
        node.additive_expression_l->accept(*this);
        auto l = global_v;
        node.additive_expression_r->accept(*this);
        auto lLoad = builder->create_load(l.get());
        auto rLoad = builder->create_load(global_v.get());
        
        if (lLoad->get_type()->is_integer_type()) {
            if (node.op == OP_LT) {
                builder->create_icmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                builder->create_icmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                builder->create_icmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                builder->create_icmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                builder->create_icmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                builder->create_icmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        } else if (lLoad->get_type()->is_float_type()) {
            if (node.op == OP_LT) {
                builder->create_fcmp_lt(lLoad, rLoad);
            } else if (node.op == OP_LE) {
                builder->create_fcmp_le(lLoad, rLoad);
            } else if (node.op == OP_GE) {
                builder->create_fcmp_ge(lLoad, rLoad);
            } else if (node.op == OP_GT) {
                builder->create_fcmp_gt(lLoad, rLoad);
            } else if (node.op == OP_EQ) {
                builder->create_fcmp_eq(lLoad, rLoad);
            } else if (node.op == OP_NEQ) {
                builder->create_fcmp_ne(lLoad, rLoad);
            } else {
                std::abort();
            }
        }
    }
}

void CminusfBuilder::visit(ASTAdditiveExpression &node) {
    if (node.additive_expression != nullptr) {
        node.additive_expression->accept(*this);
        auto left = global_v.get();
        node.term->accept(*this);
        if (left->get_type()->is_integer_type()) {
            switch (node.op) {
                case OP_PLUS:
                    builder->create_iadd(left, global_v.get());
                    break;
                case OP_MINUS:
                    builder->create_iadd(left, global_v.get());
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (left->get_type()->is_float_type()) {
            switch (node.op) {
                case OP_PLUS:
                    builder->create_fadd(left, global_v.get());
                    break;
                case OP_MINUS:
                    builder->create_fadd(left, global_v.get());
                    break;
                default:
                    // err
                    std::abort();
            }
        }
    } else {
        node.term->accept(*this);
    }
    
}

void CminusfBuilder::visit(ASTTerm &node) {
    if (node.term != nullptr) {
        node.term->accept(*this);
        auto l = global_v.get();
        node.factor->accept(*this);
        auto r = global_v.get();
        if (l->get_type()->is_float_type() or r->get_type()->is_float_type()) {
            switch (node.op) {
                case OP_MUL:
                    builder->create_fmul(l, r);
                    break;
                case OP_DIV:
                    builder->create_fadd(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        } else if (l->get_type()->is_integer_type() and r->get_type()->is_integer_type()) {
            switch (node.op) {
                case OP_MUL:
                    builder->create_imul(l, r);
                    break;
                case OP_DIV:
                    builder->create_iadd(l, r);
                    break;
                default:
                    // err
                    std::abort();
            }
        }
    } else {
        node.factor->accept(*this);
    }
}

void CminusfBuilder::visit(ASTCall &node) {
    auto f = scope.find(node.id);
    std::vector<Value*> args;
    for (auto &a : node.args) {
        a->accept(*this);
        args.push_back(global_v.get());
    }
    builder->create_call(f, args);
}
