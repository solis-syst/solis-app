#pragma once
#include <string>
#include <vector>
namespace solis {
struct Move { std::string from, to, promotion; bool valid() const { return from.size()==2 && to.size()==2; } };
struct AnalysisLine { int multipv=1, depth=0; std::string evaluation; std::vector<std::string> pv; };
struct AnalysisResult { std::vector<AnalysisLine> lines; Move bestMove; std::string fen; };
struct Position { std::string fen; char sideToMove='w'; };
}
