import json, os, sys
analysis_path = "/home/rainyu/Study/Project/Python/AGENT_OPENCODE/ICE/IDCBench/AV/svp_simple_001/improved_interrupt_analysis"
sys.path.insert(0, "/home/rainyu/Study/Project/Python/AGENT_OPENCODE/ICE/ice")
from ice_core.static_analysis.extractors.shared_var import identify_shared_variables

analysis_results = {}
for f in ["functions", "global_variables", "function_call_graph", "variable_operations", "interrupt_switches", "interrupt_priorities", "shared_variables"]:
    f_path = os.path.join(analysis_path, f"{f}.json")
    if os.path.exists(f_path):
        with open(f_path, 'r', encoding='utf-8') as fh:
            analysis_results[f] = json.load(fh)
    else:
        analysis_results[f] = []

new_shared = identify_shared_variables(analysis_results)
print(json.dumps(new_shared, indent=2))
