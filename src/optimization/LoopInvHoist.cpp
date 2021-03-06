#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

void LoopInvHoist::retmovfinal(BasicBlock* bb)
{
    Instruction* instr=nullptr;
    for (auto i:bb->get_instructions())
        if (i->isTerminator())
            instr=i;
    if (instr==nullptr)
        return;
    if (instr->is_br())
    {
        BranchInst* brinstr=static_cast<BranchInst*>(instr);
        BranchInst* newbrinstr=nullptr;
        if (brinstr->is_cond_br())
        {
            auto op0=brinstr->get_operand(0);
            auto op1=brinstr->get_operand(1);
            auto op2=brinstr->get_operand(2);
            newbrinstr=brinstr->create_cond_br(op0,static_cast<BasicBlock*>(op1),static_cast<BasicBlock*>(op2),bb);
        }
        else
        {
            auto op0=brinstr->get_operand(0);
            newbrinstr=brinstr->create_br(static_cast<BasicBlock*>(op0),bb);
        }
        rmlist.push_back(brinstr);
    }
    if (instr->is_ret())
    {
        ReturnInst* retinstr=static_cast<ReturnInst*>(instr);
        ReturnInst* newretinstr=nullptr;
        if (retinstr->is_void_ret())
            newretinstr=retinstr->create_void_ret(bb);
        else
        {
            auto op0=retinstr->get_operand(0);
            newretinstr=retinstr->create_ret(op0,bb);
        }
        rmlist.push_back(retinstr);
    }
}

void LoopInvHoist::moveinstr(Instruction* instr,BasicBlock* bb)
{
    if (instr->isBinary())
    {
        change=true;
        BinaryInst* bininstr=static_cast<BinaryInst*>(instr);
        BinaryInst* newbininstr=nullptr;
        auto op0=bininstr->get_operand(0);
        auto op1=bininstr->get_operand(1);
        if (bininstr->is_add())  
            newbininstr=bininstr->create_add(op0,op1,bb,m_);
        if (bininstr->is_fadd())  
            newbininstr=bininstr->create_fadd(op0,op1,bb,m_);
        if (bininstr->is_sub())  
            newbininstr=bininstr->create_sub(op0,op1,bb,m_);
        if (bininstr->is_fsub())  
            newbininstr=bininstr->create_fsub(op0,op1,bb,m_);
        if (bininstr->is_mul())  
            newbininstr=bininstr->create_mul(op0,op1,bb,m_);
        if (bininstr->is_fmul())  
            newbininstr=bininstr->create_fmul(op0,op1,bb,m_);
        if (bininstr->is_div())  
            newbininstr=bininstr->create_sdiv(op0,op1,bb,m_);
        if (bininstr->is_fdiv())  
            newbininstr=bininstr->create_fdiv(op0,op1,bb,m_);
        newbininstr->set_name(bininstr->get_name());
        rmlist.push_back(bininstr);
        retmovfinal(bb);
    }
    if (instr->is_si2fp())
    {
        change=true;
        SiToFpInst* si2fpinstr=static_cast<SiToFpInst*>(instr);
        SiToFpInst* newsi2fpinstr=nullptr;
        auto op0=si2fpinstr->get_operand(0);
        auto ty=si2fpinstr->get_dest_type();
        newsi2fpinstr=si2fpinstr->create_sitofp(op0,ty,bb);
        newsi2fpinstr->set_name(si2fpinstr->get_name());
        rmlist.push_back(si2fpinstr);
        retmovfinal(bb);
    }
    if (instr->is_fp2si())
    {
        change=true;
        FpToSiInst* fp2siinstr=static_cast<FpToSiInst*>(instr);
        FpToSiInst* newfp2siinstr=nullptr;
        auto op0=fp2siinstr->get_operand(0);
        auto ty=fp2siinstr->get_dest_type();
        newfp2siinstr=fp2siinstr->create_fptosi(op0,ty,bb);
        newfp2siinstr->set_name(fp2siinstr->get_name());
        rmlist.push_back(fp2siinstr);
        retmovfinal(bb);
    }
    if (instr->is_zext())
    {
        change=true;
        ZextInst* zinstr=static_cast<ZextInst*>(instr);
        ZextInst* newzinstr=nullptr;
        auto op0=zinstr->get_operand(0);
        auto ty=zinstr->get_dest_type();
        newzinstr=zinstr->create_zext(op0,ty,bb);
        newzinstr->set_name(zinstr->get_name());
        rmlist.push_back(zinstr);
        retmovfinal(bb);
    }
}

void LoopInvHoist::find_right_no_assign(std::unordered_set<BBset_t *>::iterator bbs)
{
    for (auto i:*(*bbs))
        for (auto iinstr:i->get_instructions())
            for (auto iop:iinstr->get_operands())
                for (auto j:*(*bbs))
                    for (auto jinstr:j->get_instructions())
                        if (iop==jinstr)
                            rightnoassign[iinstr]=false;
}

void LoopInvHoist::move_all_instr(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* base)
{
    BasicBlock* movedest=nullptr;
    for (auto i:base->get_pre_basic_blocks())
    {
        bool notinloop=true;
        for (auto j:*(*bbs))
            if (i==j)
            {
                notinloop=false;
                break;
            }
        if (notinloop)
        {
            movedest=i;
            break;
        }
    }
    for (auto i:*(*bbs))
        for (auto iinstr:i->get_instructions())
            if (rightnoassign[iinstr])
                moveinstr(iinstr,movedest);
    for (auto i:rmlist)
        i->get_parent()->delete_instr(i);
}

void LoopInvHoist::run()
{
    m_->set_print_name();
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();
    for (auto bbs=loop_searcher.begin();bbs!=loop_searcher.end();bbs++)
    {
        change=true;
        while (change)
        {
            change=false;
            auto base=loop_searcher.get_loop_base(*bbs);
            for (auto bb:*(*bbs))
                for (auto instr:bb->get_instructions())
                    rightnoassign[instr]=true;
            rmlist.clear();
            find_right_no_assign(bbs);
            move_all_instr(bbs,base);
        }
    }
}