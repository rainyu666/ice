import os, sys, json
sys.path.insert(0, "/home/rainyu/Study/Project/Python/AGENT_OPENCODE/ICE/ice")
from ice_core.static_analysis.analyzer import ImprovedInterruptModelAnalyzer

analyzer = ImprovedInterruptModelAnalyzer("/home/rainyu/Study/Project/Python/AGENT_OPENCODE/ICE/IDCBench/AV/svp_simple_001")
analyzer.analyze_project(debug_mode=False)

with open("/home/rainyu/Study/Project/Python/AGENT_OPENCODE/ICE/IDCBench/AV/svp_simple_001/improved_interrupt_analysis/shared_variables.json") as f:
    print(json.load(f))
