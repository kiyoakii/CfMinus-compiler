#include "ConstPropagation.hpp"
#include "logging.hpp"

// 给出了返回整形值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式
ConstantInt *ConstFolder::compute(
        Instruction::OpID op,
        ConstantInt *value1,
        ConstantInt *value2) {

    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
        case Instruction::add:
            return ConstantInt::get(c_value1 + c_value2, module_);
            break;
        case Instruction::sub:
            return ConstantInt::get(c_value1 - c_value2, module_);
            break;
        case Instruction::mul:
            return ConstantInt::get(c_value1 * c_value2, module_);
            break;
        case Instruction::sdiv:
            return ConstantInt::get((int) (c_value1 / c_value2), module_);
            break;
        default:
            return nullptr;
            break;
    }
}

//浮点型的折叠
ConstantFP *ConstFolder::compute(
        Instruction::OpID op,
        ConstantFP *value1,
        ConstantFP *value2) {

    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op) {
        case Instruction::fadd:
            return ConstantFP::get(c_value1 + c_value2, module_);
            break;
        case Instruction::fsub:
            return ConstantFP::get(c_value1 - c_value2, module_);
            break;
        case Instruction::fmul:
            return ConstantFP::get(c_value1 * c_value2, module_);
            break;
        case Instruction::fdiv:
            return ConstantFP::get(c_value1 / c_value2, module_);
            break;
        default:
            return nullptr;
            break;
    }
}

//比较运算符（标准）
ConstantInt *ConstFolder::compute(
        CmpInst::CmpOp op,
        ConstantInt *value1,
        ConstantInt *value2
) {
    int c_value1 = value1->get_value();
    int c_value2 = value2->get_value();
    switch (op) {
        case CmpInst::EQ:
            return ConstantInt::get(c_value1 == c_value2, module_);
            break;
        case CmpInst::NE:
            return ConstantInt::get(c_value1 != c_value2, module_);
            break;
        case CmpInst::GT:
            return ConstantInt::get(c_value1 > c_value2, module_);
            break;
        case CmpInst::GE:
            return ConstantInt::get(c_value1 >= c_value2, module_);
            break;
        case CmpInst::LT:
            return ConstantInt::get(c_value1 < c_value2, module_);
            break;
        case CmpInst::LE:
            return ConstantInt::get(c_value1 <= c_value2, module_);
            break;
        default:
            return nullptr;
            break;
    }
}

//比较运算符（浮点型）
ConstantFP *ConstFolder::compute(
        FCmpInst::CmpOp op,
        ConstantFP *value1,
        ConstantFP *value2
) {
    float c_value1 = value1->get_value();
    float c_value2 = value2->get_value();
    switch (op) {
        case CmpInst::EQ:
            return ConstantFP::get(c_value1 == c_value2, module_);
            break;
        case CmpInst::NE:
            return ConstantFP::get(c_value1 != c_value2, module_);
            break;
        case CmpInst::GT:
            return ConstantFP::get(c_value1 > c_value2, module_);
            break;
        case CmpInst::GE:
            return ConstantFP::get(c_value1 >= c_value2, module_);
            break;
        case CmpInst::LT:
            return ConstantFP::get(c_value1 < c_value2, module_);
            break;
        case CmpInst::LE:
            return ConstantFP::get(c_value1 <= c_value2, module_);
            break;
        default:
            return nullptr;
            break;
    }
}

// 用来判断value是否为ConstantFP，如果不是则会返回nullptr
ConstantFP *cast_constantfp(Value *value) {
    auto constant_fp_ptr = dynamic_cast<ConstantFP *>(value);
    if (constant_fp_ptr) {
        return constant_fp_ptr;
    } else {
        return nullptr;
    }
}

ConstantInt *cast_constantint(Value *value) {
    auto constant_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (constant_int_ptr) {
        return constant_int_ptr;
    } else {
        return nullptr;
    }
}


void ConstPropagation::run() {
    auto constFolder = new ConstFolder(m_);
    for (auto func : m_->get_functions()) {
        for (auto bb : func->get_basic_blocks()) {
            std::vector<Instruction *> wait_delete;
            std::map<Value *,Value *> global_value;
            for (auto instr : bb->get_instructions()) {
                if (instr->is_add() || instr->is_sub() || instr->is_mul() || instr->is_div()) {
                    auto binary_instr = dynamic_cast<BinaryInst *>(instr);
                    auto v1 = cast_constantint(instr->get_operand(0));
                    auto v2 = cast_constantint(instr->get_operand(1));
                    if (v1 != nullptr && v2 != nullptr) {
                        auto flod_const = constFolder->compute(binary_instr->get_instr_type(), v1, v2);
                        instr->replace_all_use_with(flod_const);
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_fadd() || instr->is_fsub() || instr->is_fmul() || instr->is_fdiv()) {
                    auto binary_instr = dynamic_cast<BinaryInst *>(instr);
                    auto v1 = cast_constantfp(instr->get_operand(0));
                    auto v2 = cast_constantfp(instr->get_operand(1));
                    if (v1 != nullptr && v2 != nullptr) {
                        auto flod_const = constFolder->compute(binary_instr->get_instr_type(), v1, v2);
                        instr->replace_all_use_with(flod_const);
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_cmp()) {
                    auto cmp_instr = dynamic_cast<CmpInst *>(instr);
                    auto v1 = cast_constantint(instr->get_operand(0));
                    auto v2 = cast_constantint(instr->get_operand(1));
                    if (v1 != nullptr && v2 != nullptr) {
                        auto flod_const = constFolder->compute(cmp_instr->get_cmp_op(), v1, v2);
                        instr->replace_all_use_with(flod_const);
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_fcmp()) {
                    auto cmp_instr = dynamic_cast<FCmpInst *>(instr);
                    auto v1 = cast_constantfp(instr->get_operand(0));
                    auto v2 = cast_constantfp(instr->get_operand(1));
                    if (v1 != nullptr && v2 != nullptr) {
                        auto flod_const = constFolder->compute(cmp_instr->get_cmp_op(), v1, v2);
                        instr->replace_all_use_with(flod_const);
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_fp2si()) {
                    auto fp2si_instr = dynamic_cast<FpToSiInst *>(instr);
                    ConstantFP *val = cast_constantfp(fp2si_instr->get_operand(0));
                    if (val != nullptr) {
                        instr->replace_all_use_with(ConstantInt::get((int) val->get_value(), m_));
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_si2fp()) {
                    auto si2fp_instr = dynamic_cast<SiToFpInst *>(instr);
                    ConstantInt *val = cast_constantint(si2fp_instr->get_operand(0));
                    if (val != nullptr) {
                        instr->replace_all_use_with(ConstantFP::get((float) val->get_value(), m_));
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_zext()) {
                    auto zext_instr = dynamic_cast<ZextInst *>(instr);
                    ConstantInt *val = cast_constantint(zext_instr->get_operand(0));
                    if (val != nullptr) {
                        instr->replace_all_use_with(ConstantInt::get((int) val->get_value(), m_));
                        wait_delete.push_back(instr);
                    }
                } else if (instr->is_store()){
                    auto store_instr = dynamic_cast<StoreInst *>(instr);
                    global_value[store_instr->get_lval()] = store_instr->get_rval();
                } else if (instr->is_load()){
                    auto load_instr = dynamic_cast<LoadInst*>(instr);
                    if(global_value.count(load_instr->get_lval())){
                        instr->replace_all_use_with(global_value[(load_instr->get_lval())]);
                        wait_delete.push_back(instr);
                    }
                } else if (bb->get_terminator()->is_br()) {
                    auto br = bb->get_terminator();
                    if (dynamic_cast<BranchInst *>(br)->is_cond_br()) {
                        auto cond = dynamic_cast<ConstantInt *>(br->get_operand(0));//??float类型怎么办
                        auto truebb = dynamic_cast<BasicBlock *>(br->get_operand(1));
                        auto falsebb = dynamic_cast<BasicBlock *>(br->get_operand(2));
                        if (cond) {
                            if (cond->get_value() == 0) {
                                BranchInst::create_br(falsebb, bb);
                                wait_delete.push_back(instr);
                                truebb->remove_pre_basic_block(bb);
                            } else {
                                BranchInst::create_br(truebb, bb);
                                wait_delete.push_back(instr);
                                falsebb->remove_pre_basic_block(bb);

                            }
                        }
                    }
                }
            }
            for (auto instr : wait_delete) {
                bb->delete_instr(instr);
            }
        }
    }
    for (auto func : m_->get_functions()) {
        while (true) {
            bool changed = false;
            std::vector<BasicBlock *> wait_delete;
            for (auto bb : func->get_basic_blocks()) {
                if (bb->get_name() == "label_entry")continue;
                if (bb->get_pre_basic_blocks().empty()) {
                    wait_delete.push_back(bb);
                    changed = true;
                }
            }
            for( auto bb:wait_delete )
            {
                bb->erase_from_parent();
            }
            if (!changed)break;
        }

    }

}
