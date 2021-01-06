#include "ActiveVars.hpp"

bool is_var(Value *val) {
    return val->get_name().length() != 0;
}

void ActiveVars::buildDFSList(Function * func) {
    visited.clear();
    for (auto bb : func->get_basic_blocks()) {
        visited[bb] = false;
    }
    for (auto bb : func->get_basic_blocks()) {
        if (!visited[bb]) {
            DFSVisit(bb);
        }
    }
}

void ActiveVars::DFSVisit(BasicBlock* bb) {
    DFSList.push_back(bb);
    visited[bb] = true;
    for (auto sbb : bb->get_succ_basic_blocks()) {
        if (!visited[sbb]) {
            DFSVisit(sbb);
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
            def.clear();
            use.clear();
            phi_in.clear();
            phi_var.clear();

            buildDFSList(func_);

            // 在此分析 func_ 的每个 bb 块的活跃变量，并存储在 live_in live_out 结构内
            // 思路：
            // 对每个 bb，得到 use 和 def
            // 按 DFS 逆序，更新 live_in 和 live_out
            for (auto bb : func_->get_basic_blocks()) {
                auto &bb_def = def[bb];
                auto &bb_use = use[bb];
                std::set<Value *> used, defined;
                for (auto instr : bb->get_instructions()) {
                    // build use
                    if (instr->is_phi()) {
                        for (size_t i = 0; i < instr->get_num_operand(); i += 2) {
                            auto op = instr->get_operand(i);
                            auto pbb = dynamic_cast<BasicBlock *>(instr->get_operand(i + 1));
                            if (defined.find(op) == defined.end() && pbb != nullptr && op != nullptr && is_var(op)) {
                                used.insert(op);
                                phi_in[bb].insert(op);
                                phi_var[pbb].insert(op);
                            }
                        }
                        if (!instr->is_void()) {
                            defined.insert(instr);
                            if (used.find(instr) == used.end()) {
                                bb_def.insert(instr);
                            }
                        }
                        continue;
                    }
                    for (auto op : instr->get_operands()) {
                        if (!is_var(op) || op->get_type()->is_label_type()
                            || op->get_type()->is_function_type()) {
                            continue;
                        }
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

            bool fixed;
            do {
                fixed = true;
                for (auto _bb = DFSList.rbegin(); _bb != DFSList.rend(); _bb++) {
                    auto bb = *_bb;
                    for (auto sbb : bb->get_succ_basic_blocks()) {
                        for (auto e : live_in[sbb]) {
                            live_out[bb].insert(e);
                        }
                        for (auto e : phi_var[bb]) {
                            live_out[bb].insert(e);
                        }
                    }
                    auto new_live_in = live_out[bb];
                    for (auto e : def[bb]) {
                        if (new_live_in.find(e) != live_in[bb].end()) {
                            new_live_in.erase(e);
                        }
                    }
                    for (auto e : use[bb]) {
                        new_live_in.insert(e);
                    }
                    if (new_live_in != live_in[bb]) {
                        fixed = false;
                        live_in[bb] = new_live_in;
                    }
                }
            } while (!fixed);

            for (auto bb : func_->get_basic_blocks()) {
                for (auto e : phi_in[bb]) {
                    live_in[bb].insert(e);
                }
            }

            output_active_vars << print();
            output_active_vars << ",";
        }
    }
    output_active_vars << "]";
    output_active_vars.close();
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