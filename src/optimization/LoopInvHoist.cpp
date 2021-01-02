#include <algorithm>
#include "logging.hpp"
#include "LoopSearch.hpp"
#include "LoopInvHoist.hpp"

bool change;

void LoopInvHoist::retmovfinal(BasicBlock* bb)
{
    std::cout<<"move terminal of bb: "<<bb->get_name()<<std::endl;
    Instruction* instr=nullptr;
    for (auto i:bb->get_instructions())
    {
        if (i->isTerminator())
            instr=i;
    }
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
        {
            newretinstr=retinstr->create_void_ret(bb);
        }
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
        BinaryInst* bininstr=nullptr;
        BinaryInst* newbininstr=nullptr;
        std::cout<<"move "<<instr->get_name()<<" to "<<bb->get_name()<<std::endl;
        change=true;
        bininstr=static_cast<BinaryInst*>(instr);
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
        std::cout<<"instr push in rmlist"<<std::endl;
        retmovfinal(bb);
    }
}

/*void LoopInvHoist::f1(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* bb,bool print)
{
    bbvisit[bb]=true;
    std::cout<<"bb: "<<bb->get_name()<<std::endl;
    for (auto nextbb:bb->get_succ_basic_blocks())
    {
        bool inloop=false;
        if (bbformer[nextbb][bb]==true)
            continue;
        for (auto i:*(*bbs))
        {
            if ((i==nextbb) && (!bbvisit[nextbb]))
            {
                inloop=true;
                break;
            }
        }
        if (inloop)
        {
            bbformer[nextbb][bb]=true;
            for (auto i:*(*bbs))
            {
                bbformer[nextbb][i]=bbformer[nextbb][i] && bbformer[bb][i];
            }
            f1(bbs,nextbb,false);
        }
    }
}

void LoopInvHoist::f2(std::unordered_set<BBset_t *>::iterator bbs,bool print)
{
    for (auto i:*(*bbs))
    {
        //std::cout<<"bb: "<<i->get_name()<<std::endl;
        for (auto iinstr:i->get_instructions())
        {
            if (!leftnouse[iinstr])
                continue;
            for (auto j:*(*bbs))
            {
                if (!bbformer[i][j])
                    continue;
                for (auto jinstr:j->get_instructions())
                {
                    for (auto jop:jinstr->get_operands())
                    {
                        if (iinstr==jop)
                            leftnouse[iinstr]=false;
                    }
                }
            }
            //if (leftnouse[iinstr])
            //    std::cout<<iinstr->get_name()<<" not used"<<std::endl;
            //else
            //    std::cout<<iinstr->get_name()<<" used"<<std::endl;
        }
    }
}*/

void LoopInvHoist::f3(std::unordered_set<BBset_t *>::iterator bbs,bool print)
{
    for (auto i:*(*bbs))
    {
        if (print)  std::cout<<"bb: "<<i->get_name()<<std::endl;
        for (auto iinstr:i->get_instructions())
        {
            for (auto iop:iinstr->get_operands())
            {
                for (auto j:*(*bbs))
                {
                    for (auto jinstr:j->get_instructions())
                    {
                        if (iop==jinstr)
                        {
                            rightnoassign[iinstr]=false;
                        }
                    }
                }
            }
            if (rightnoassign[iinstr])
                if (print)  std::cout<<iinstr->get_name()<<" right not assigned"<<std::endl;
            else
                if (print)  std::cout<<iinstr->get_name()<<" right assigned"<<std::endl;
        }
    }
}

void LoopInvHoist::f4(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* bb,bool print)
{
    BasicBlock* movedest=nullptr;
    for (auto i:bb->get_pre_basic_blocks())
    {
        bool notinloop=true;
        for (auto j:*(*bbs))
        {
            if (i==j)
            {
                notinloop==false;
                break;
            }
        }
        if (notinloop)
        {
            movedest=i;
            break;
        }
    }
    for (auto i:*(*bbs))
    {
        std::cout<<"bb: "<<i->get_name()<<std::endl;
        for (auto iinstr:i->get_instructions())
        {
            if (rightnoassign[iinstr])
            {
                moveinstr(iinstr,movedest);
            }
        }
    }
    std::cout<<"all rmlist found"<<std::endl;
    for (auto i:rmlist)
    {
        i->get_parent()->delete_instr(i);
    }
    std::cout<<"all removed"<<std::endl;
}

void LoopInvHoist::run()
{
    m_->set_print_name();
    LoopSearch loop_searcher(m_, false);
    loop_searcher.run();
    for (auto fun:m_->get_functions())
    {
        std::cout<<fun->get_name()<<std::endl;
    }
    int lv=0;
    for (auto bbs=loop_searcher.begin();bbs!=loop_searcher.end();bbs++)
    {
        change=true;
        lv++;
        std::cout<<"loop"<<lv<<std::endl;
        while (change)
        {
            change=false;
            auto bb=loop_searcher.get_loop_base(*bbs);
            for (auto bb:*(*bbs))
            {
                bbvisit[bb]=false;
                for (auto bbb:*(*bbs))
                    bbformer[bbb][bb]=false;
                for (auto instr:bb->get_instructions())
                {
                    leftnouse[instr]=true;
                    rightnoassign[instr]=true;
                }
            }
            rmlist.clear();
            //std::cout<<"f1*****"<<std::endl;
            ///f1(bbs,bb);
            //std::cout<<"f2*****"<<std::endl;
            //f2(bbs);
            f3(bbs,false);
            std::cout<<"f4*****"<<std::endl;
            f4(bbs,bb,true);
        }
    }
}