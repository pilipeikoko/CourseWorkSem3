#include <iostream>
#include <vector>
#include "algorithm"
#include <string>
#include "cpp/sc_addr.hpp"
#include "cpp/sc_memory.hpp"
#include "cpp/sc_iterator.hpp"
#include "utils.h"

ScAddr rrel_arcs, rrel_nodes, nrel_weight;

class Error{
    std::string message;
public:
    Error(std::string message): message(message){};
    std::string get_message() { return this->message;}
};

ScMemory initialize_memory(char* path_to_kb_bin,char* path_to_sc_web_ini,char* path_to_sc_memory);

ScAddr get_system_identifier(const std::unique_ptr<ScMemoryContext> &context, std::string identifier);
ScAddr get_all_arcs(const std::unique_ptr<ScMemoryContext> &context,ScAddr graph);
ScAddr get_all_vertexes(const std::unique_ptr<ScMemoryContext> &context,ScAddr graph);
ScAddr find_addr_of_arc_in_set(const std::unique_ptr<ScMemoryContext> &context, ScAddr source_of_arc, ScAddr target_of_arc, ScAddr set_of_arcs) throw (Error);
ScAddr get_addr_of_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc);

int get_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc) throw (Error);
int deep_search(const std::unique_ptr<ScMemoryContext> &context, ScAddr source, ScAddr arcs, int curr_weight, ScAddr temp_source,ScAddrVector &visited_nodes);
int find_eccentricity(const std::unique_ptr<ScMemoryContext> &context, ScAddr set_of_arcs, ScAddr source);
int find_min_in_vec(std::vector<int> vector);
int ScAddr_to_int(const std::unique_ptr<ScMemoryContext> &context, ScAddr addr);

bool edge_in_set_exists(const std::unique_ptr<ScMemoryContext> &context, ScAddr source_of_arc, ScAddr target_of_arc, ScAddr arcs);
bool is_in_vector(const std::unique_ptr<ScMemoryContext> &context, ScAddr to_search, ScAddrVector vector);

void print_all_arcs(const std::unique_ptr<ScMemoryContext> &context, ScAddr arcs);
void add_edge_with_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr source, ScAddr &arcs, int new_weight, ScAddr target) throw (Error);
void run_test(const std::unique_ptr<ScMemoryContext> &context, std::string number_of_test);
void clear_memory(const std::unique_ptr<ScMemoryContext> &context, ScAddr arcs, ScAddr vertexes);

std::string print_arc_info(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc);
std::string print_vertex_info(const std::unique_ptr<ScMemoryContext> &context, ScAddr vertex);

ScMemory initialize_memory(char* path_to_kb_bin,char* path_to_sc_web_ini,char* path_to_sc_machine_bin_extensions){
    sc_memory_params params;
    sc_memory_params_clear(&params);
    params.repo_path = path_to_kb_bin;
    params.config_file = path_to_sc_web_ini;
    params.ext_path = path_to_sc_machine_bin_extensions;
    params.clear = SC_FALSE;
    ScMemory mem;
    mem.Initialize(params);
    return mem;
}

ScAddr get_system_identifier(const std::unique_ptr<ScMemoryContext> &context, std::string identifier){
    return context->HelperResolveSystemIdtf(identifier);
}

ScAddr get_all_arcs(const std::unique_ptr<ScMemoryContext> &context,ScAddr graph) {
    ScAddr all_arcs = context->CreateNode(ScType::Const);
    ScIterator5Ptr iter = context->Iterator5(graph, ScType::EdgeAccessConstPosPerm, ScType(0),
                                             ScType::EdgeAccessConstPosPerm, rrel_arcs);

    if (iter->Next()) {
        ScAddr set_arcs = iter->Get(2);
        ScIterator3Ptr iter_arc = context->Iterator3(set_arcs, ScType::EdgeAccessConstPosPerm, ScType(0));
        while (iter_arc->Next()) {
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, all_arcs, iter_arc->Get(2));
        }
    }
    return all_arcs;
}

ScAddr get_all_vertexes(const std::unique_ptr<ScMemoryContext> &context, ScAddr graph) {
    ScAddr all_vertexes = context->CreateNode(ScType::Const);
    ScIterator5Ptr iter = context->Iterator5(graph, ScType::EdgeAccessConstPosPerm, ScType(0),
                                             ScType::EdgeAccessConstPosPerm, rrel_nodes);

    if (iter->Next()) {
        ScAddr set_vertexes = iter->Get(2);
        ScIterator3Ptr it_vertex = context->Iterator3(set_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
        while (it_vertex->Next()) {
            ScAddr curr_vertex = it_vertex->Get(2);
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, all_vertexes, curr_vertex);
        }

    }
    return all_vertexes;
}

ScAddr find_addr_of_arc_in_set(const std::unique_ptr<ScMemoryContext> &context, ScAddr source_of_arc, ScAddr target_of_arc,
                               ScAddr set_of_arcs) throw (Error) {
    ScIterator3Ptr iter = context->Iterator3(set_of_arcs, ScType::EdgeAccessConstPosPerm, ScType(0));

    while (iter->Next()) {
        if (context->GetEdgeSource(iter->Get(2)) == source_of_arc && context->GetEdgeTarget(iter->Get(2)) == target_of_arc)
            return iter->Get(2);
    }

    throw Error ("Arc not found\n");
}

ScAddr get_addr_of_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc) {
    ScIterator5Ptr iter = context->Iterator5(arc, ScType::EdgeDCommon, ScType(0), ScType::EdgeAccessConstPosPerm,
                                             nrel_weight);
    if (iter->Next()) {
        return iter->Get(2);
    }
    throw Error("Arc not found\n");
}

int get_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc) throw(Error) {
    ScAddr node_weight, weight;
    ScIterator5Ptr it = context->Iterator5(arc, ScType::EdgeDCommon, ScType(0), ScType::EdgeAccessConstPosPerm,
                                           nrel_weight);
    if (it->Next()) {
        return stoi(context->HelperGetSystemIdtf(it->Get(2)));
    }
    throw Error("Arc not found\n");
}

int deep_search(const std::unique_ptr<ScMemoryContext> &context, ScAddr source, ScAddr arcs, int curr_weight,
                ScAddr temp_source,ScAddrVector &visited_nodes) {
    ScIterator3Ptr iter = context->Iterator3(arcs, ScType::EdgeAccessConstPosPerm, ScType(0));

    while (iter->Next()) {
        std::string start = context->HelperGetSystemIdtf(temp_source);
        std::string sou = context->HelperGetSystemIdtf(context->GetEdgeSource(iter->Get(2)));
        std::string tar = context->HelperGetSystemIdtf(context->GetEdgeTarget(iter->Get(2)));
        if (context->GetEdgeSource(iter->Get(2)) == temp_source && !is_in_vector(context,context->GetEdgeTarget(iter->Get(2)),visited_nodes)) {
            ScAddr target = iter->Get(2);
            visited_nodes.push_back(context->GetEdgeSource(target));
            int siz=visited_nodes.size();
            curr_weight += get_weight(context, target);
                add_edge_with_weight(context, source, arcs, curr_weight, context->GetEdgeTarget(target));
            deep_search(context, source, arcs, curr_weight, context->GetEdgeTarget(iter->Get(2)),visited_nodes);
            curr_weight -= get_weight(context, target);
            visited_nodes.erase(std::find(visited_nodes.begin(),visited_nodes.end(),context->GetEdgeSource(target)));
        }
    }
    return find_eccentricity(context, arcs, source);
}

int find_eccentricity(const std::unique_ptr<ScMemoryContext> &context, ScAddr set_of_arcs, ScAddr source) {
    ScIterator3Ptr iter = context->Iterator3(set_of_arcs, ScType::EdgeAccessConstPosPerm, ScType(0));
    int max = 0;
    while (iter->Next()) {
        if (context->GetEdgeSource(iter->Get(2)) == source && get_weight(context, iter->Get(2)) > max) {
            max = get_weight(context, iter->Get(2));
        }
    }
    return max;
}

int find_min_in_vec(std::vector<int> vector){
    int min = 2147483647;
    for(auto ch:vector){
        if(ch<min && ch != 0){
            min = ch;
        }
    }
    return min;
}

int ScAddr_to_int(const std::unique_ptr<ScMemoryContext> &context, ScAddr addr) {
    return stoi(context->HelperGetSystemIdtf(addr));
}

bool edge_in_set_exists(const std::unique_ptr<ScMemoryContext> &context, ScAddr source_of_arc, ScAddr target_of_arc, ScAddr arcs) {

    ScIterator3Ptr iter = context->Iterator3(arcs, ScType::EdgeAccessConstPosPerm, ScType(0));

    while (iter->Next()) {

        if (context->GetEdgeSource(iter->Get(2)) == source_of_arc &&
            context->GetEdgeTarget(iter->Get(2)) == target_of_arc)
            return true;
    }
    return false;
}

bool is_in_vector(const std::unique_ptr<ScMemoryContext> &context, ScAddr to_search, ScAddrVector vector){
    for(auto ch : vector){
        if(ch == to_search)
            return true;
    }
    return false;
}

void print_all_arcs(const std::unique_ptr<ScMemoryContext> &context, ScAddr arcs) {
    ScIterator3Ptr iter = context->Iterator3(arcs, ScType::EdgeAccessConstPosPerm, ScType(0));
    while (iter->Next()) {
        std::cout << get_weight(context, iter->Get(2)) << "  " << print_arc_info(context, iter->Get(2));
    }
}

void add_edge_with_weight(const std::unique_ptr<ScMemoryContext> &context, ScAddr source, ScAddr &arcs,
                          int new_weight, ScAddr target) throw (Error) {
    if (!edge_in_set_exists(context, source, target, arcs)) {
        ScAddr added_arc = context->CreateEdge(ScType::EdgeDCommon, source, target);
        context->CreateEdge(ScType::EdgeAccessConstPosPerm, arcs, added_arc);

        std::string string_weight = std::to_string(new_weight);
        ScAddr weight_node = get_system_identifier(context, string_weight);
        if(weight_node.IsValid()){
            std::string a = context->HelperGetSystemIdtf(weight_node);
            ScAddr from_arc_to_weight = context->CreateEdge(ScType::EdgeDCommon, added_arc, weight_node);
            context->CreateEdge(ScType::EdgeAccessConstPosPerm, nrel_weight, from_arc_to_weight);
        }
        else{
            ScAddr weight_to_add = context->CreateNode(ScType::Const);
            if (context->HelperSetSystemIdtf(string_weight, weight_to_add)) {
                std::string a = context->HelperGetSystemIdtf(weight_to_add);
                ScAddr from_weight_to_arc = context->CreateEdge(ScType::EdgeDCommon, added_arc, weight_to_add);
                context->CreateEdge(ScType::EdgeAccessConstPosPerm, nrel_weight, from_weight_to_arc);

            }
            else {
                throw Error("Sc memory error, couldn't set idtf\n");
            }
        }
    }
    else {
        int current_weight = get_weight(context, find_addr_of_arc_in_set(context, source,
                                                                         target, arcs));
        if (current_weight <= new_weight && current_weight!=0)
            return;
        context->EraseElement(find_addr_of_arc_in_set(context, source,
                                                      target,
                                                      arcs));

        add_edge_with_weight(context, source, arcs, new_weight, target);

    }
}

void run_test(const std::unique_ptr<ScMemoryContext> &context, std::string number_of_test) {
    std::string gr = "Gx";

    gr[1] = number_of_test[0];
    ScAddr graph = context->HelperResolveSystemIdtf(gr);
    rrel_arcs = context->HelperResolveSystemIdtf("rrel_arcs");
    rrel_nodes = context->HelperResolveSystemIdtf("rrel_nodes");
    nrel_weight = context->HelperResolveSystemIdtf("nrel_weight");

    ScAddr all_vertexes = get_all_vertexes(context,graph);
    ScAddr all_arcs = get_all_arcs(context,graph);

    std::cout<<gr<<std::endl;
    print_all_arcs(context,all_arcs);

    ScIterator3Ptr iter = context->Iterator3(all_vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));

    ScAddrVector visited_vertexes;
    std::vector<int> eccentricities;

    while (iter->Next()) {
        visited_vertexes.push_back(iter->Get(2));
        eccentricities.push_back(deep_search(context, iter->Get(2), all_arcs, 0, iter->Get(2), visited_vertexes));
        visited_vertexes.clear();
    }

    int min = find_min_in_vec(eccentricities);
    if(min == 2147483647){
        std::cout<<"Radius doesn't exist\n";
    }
    else
        std::cout<<"Radius of graph " <<gr<<" "<<min<<std::endl<<std::endl;

    context->EraseElement(all_vertexes);
    context->EraseElement(all_arcs);
}


void clear_memory(const std::unique_ptr<ScMemoryContext> &context, ScAddr arcs, ScAddr vertexes){
    ScIterator3Ptr it = context->Iterator3(arcs, ScType::EdgeAccessConstPosPerm, ScType(0));
    while (it->Next()){
        context->EraseElement(it->Get(2));
    }

    ScIterator3Ptr iter = context->Iterator3(vertexes, ScType::EdgeAccessConstPosPerm, ScType(0));
    while (iter->Next()){
        context->EraseElement(iter->Get(2));
    }
}

std::string print_arc_info(const std::unique_ptr<ScMemoryContext> &context, ScAddr arc) {
    return context->HelperGetSystemIdtf(context->GetEdgeSource(arc)) + " => " + context->HelperGetSystemIdtf(context->GetEdgeTarget(arc)) + "\n";
}

std::string print_vertex_info(const std::unique_ptr<ScMemoryContext> &context, ScAddr vertex) {
    return context->HelperGetSystemIdtf(vertex) + "\n";
}

int main()
{
    char* path_to_kb_bin = "/home/valentin/1/ostis-web-platform/kb.bin";
    char* path_to_sc_web_ini = "/home/valentin/1/ostis-web-platform/config/sc-web.ini";
    char* path_to_sc_machine_bin_extensions = "/home/valentin/1/ostis-web-platform/sc-machine/bin/extensions";

    ScMemory mem = initialize_memory(path_to_kb_bin,path_to_sc_web_ini,path_to_sc_machine_bin_extensions);

    const std::unique_ptr<ScMemoryContext> context(new ScMemoryContext(sc_access_lvl_make_max,"example"));

    run_test(context,"0");
    run_test(context,"1");
    run_test(context,"2");
    run_test(context,"3");
    run_test(context,"4");

    mem.Shutdown(true);

    return 0;
}

