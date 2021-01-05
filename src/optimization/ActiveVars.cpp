#include "ActiveVars.hpp"

void ActiveVars::buildDFSList(Function * func) {
    visited.clear();
    for (auto bb : func->get_basic_blocks()) {
        visited[bb] = false;
    }
    for (auto bb : func->get_basic_blocks()) {
        if (!visited[bb]) {
            DFSvisit(bb);
        }
    }
}

void ActiveVars::DFSvisit(BasicBlock* bb) {
    DFSList.push_back(bb);
    visited[bb] = true;
    for (auto sbb : bb->get_succ_basic_blocks()) {
        if (!visited[sbb]) {
            DFSvisit(sbb);
        }
    }
}

void ActiveVars::run()
{
    std::ofstream output_active_vars;
    output_active_vars.open("active_vars.json", std::ios::out);
    output_active_vars << "[";
    for (auto &func : this->m_->get_functions()) {
        if (func->get_basic_blocks().empty()) {
            continue;
        }
        else
        {
            func_ = func;  

            func_->set_instr_name();
            live_in.clear();
            live_out.clear();
            
            // 在此分析 func_ 的每个 bb 块的活跃变量，并存储在 live_in live_out 结构内
            // 思路：
            // 对每个 bb，得到 use 和 def
            // 按 DFS 逆序，更新 live_in 和 live_out
            for (auto bb : func_->get_basic_blocks()) {
                std::unordered_set<Value *> bb_def, bb_use;
                std::unordered_set<Value *> used, defined;
                for (auto instr : bb->get_instructions()) {
                    for (auto op : instr->get_operands()) {
                        used.insert(op);
                        if (defined.find(op) == defined.end()) {
                            bb_use.insert(op);
                        }
                    }
                    if (!instr->is_void()) {
                        defined.insert(instr);
                        if (used.find(instr) == used.end()) {
                            bb_def.insert(instr);
                        }
                    }
                }
                def[bb] = bb_def;
                use[bb] = bb_use;
            }

            // BasicBlock* EXIT = BasicBlock::create(this->m_, "__EXIT__", func);
            // def[EXIT] = std::set<Value *>();
            // use[EXIT] = std::set<Value *>();

            buildDFSList(func_);

            bool fixed = true;
            do {
                fixed = true;
                for (auto _bb = DFSList.end(); _bb != DFSList.begin(); _bb--) {
                    auto bb = *_bb;
                    auto in_bb = live_in[bb];
                    auto new_out_bb = live_out[bb];
                    auto out_bb = live_out[bb];
                    for (auto sbb : bb->get_succ_basic_blocks()) {
                        for (auto e : live_in[sbb]) {
                            new_out_bb.insert(e);
                        }
                    }
                    if (new_out_bb != out_bb) {
                        out_bb = new_out_bb;
                        fixed = false;
                    }
                    if (!fixed) {
                        for (auto pbb : bb->get_pre_basic_blocks()) {
                            live_in[bb] = live_out[bb];
                            for (auto e : def[bb]) {
                                if (live_in[bb].find(e) != live_in[bb].end()) {
                                    live_in[bb].erase(e);
                                }
                            }
                            for (auto e : use[bb]) {
                                live_in[bb].insert(e);
                            }
                        }
                    } 
                }
            } while (!fixed);

            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
    return ;
}

std::string ActiveVars::print()
{
    std::string active_vars;
    active_vars +=  "{\n";
    active_vars +=  "\"function\": \"";
    active_vars +=  func_->get_name();
    active_vars +=  "\",\n";

    active_vars +=  "\"live_in\": {\n";
    for (auto &p : live_in) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]" ;
            active_vars += ",\n";   
        }
    }
    active_vars += "\n";
    active_vars +=  "    },\n";
    
    active_vars +=  "\"live_out\": {\n";
    for (auto &p : live_out) {
        if (p.second.size() == 0) {
            continue;
        } else {
            active_vars +=  "  \"";
            active_vars +=  p.first->get_name();
            active_vars +=  "\": [" ;
            for (auto &v : p.second) {
                active_vars +=  "\"%";
                active_vars +=  v->get_name();
                active_vars +=  "\",";
            }
            active_vars += "]";
            active_vars += ",\n";
        }
    }
    active_vars += "\n";
    active_vars += "    }\n";

    active_vars += "}\n";
    active_vars += "\n";
    return active_vars;
}