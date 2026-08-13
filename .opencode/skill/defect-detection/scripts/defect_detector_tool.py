#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[4]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from ice_core.reporting import export_report  # noqa: E402


def _analysis_path(project_path: str) -> str:
    return os.path.join(project_path, "improved_interrupt_analysis")


def _state_path(project_path: str) -> Path:
    return Path(_analysis_path(project_path)) / "agent_state.json"


def _load_state(project_path: str) -> dict:
    path = _state_path(project_path)
    if not path.is_file():
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


def _save_state(project_path: str, state: dict) -> None:
    path = _state_path(project_path)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(state, ensure_ascii=False, indent=2), encoding="utf-8")


def _postprocess_analysis(analysis_path: str) -> None:
    from ice_core.pipeline.path_analysis import get_function_path_pairs
    from ice_core.pipeline.sequencer import reduce_function_variable_sequences

    get_function_path_pairs(analysis_path, save_to_file=True)
    reduce_function_variable_sequences(analysis_path)
    get_function_path_pairs(analysis_path, save_to_file=True)


def _run_analyze(project_path: str) -> dict:
    from ice_core.static_analysis.analyzer import ImprovedInterruptModelAnalyzer

    analysis_path = _analysis_path(project_path)
    if not os.path.isdir(analysis_path):
        analyzer = ImprovedInterruptModelAnalyzer(project_path)
        analyzer.analyze_project(debug_mode=False)
    _postprocess_analysis(analysis_path)
    return {"status": "ok", "analysis_path": analysis_path}


def _run_build_and_verify(project_path: str) -> tuple[dict, dict]:
    from ice_core.config import get_detection_modes, load_ice_config
    from ice_core.pipeline.detector import DefectDetector

    runtime_config = load_ice_config(str(REPO_ROOT))
    detection_modes = get_detection_modes(runtime_config)
    detector = DefectDetector(
        _analysis_path(project_path),
        detection_modes=detection_modes,
        ice_config=runtime_config,
    )
    valid_triplets, not_ready_info, *_ = detector.build_models()
    defects, others = detector.verify_defects(valid_triplets, use_agent=[False, False])
    build_result = {
        "status": "ok",
        "valid_triplets_count": len(valid_triplets),
        "needs_domain_supplement": bool(not_ready_info.get("aggregated_analysis")),
        "agent_input": (not_ready_info.get("aggregated_analysis") or {}).get("agent_input", {}),
        "summary": (not_ready_info.get("aggregated_analysis") or {}).get("summary", {}),
        "detection_modes": detection_modes,
    }
    verify_result = {
        "status": "ok",
        "defect_count": len(defects),
        "defects": [
            {
                "triplet_id": d.get("triplet_id"),
                "shared_variable": d.get("shared_variable"),
                "violation_pattern": d.get("violation_pattern"),
                "op1_line": (d.get("op1") or {}).get("line"),
                "interrupt_op_line": (d.get("interrupt_op") or {}).get("line"),
                "op3_line": (d.get("op3") or {}).get("line"),
            }
            for d in defects
        ],
        "detection_modes": detection_modes,
        "others": others,
    }
    return build_result, verify_result


def _utc_now() -> str:
    return datetime.now(timezone.utc).astimezone().isoformat(timespec="seconds")


def _next_run_record_path(project_path: str) -> Path:
    analysis_dir = Path(_analysis_path(project_path))
    analysis_dir.mkdir(parents=True, exist_ok=True)
    existing = sorted(analysis_dir.glob("opencode_run_*.md"))
    max_idx = 0
    for path in existing:
        stem = path.stem
        try:
            max_idx = max(max_idx, int(stem.split("_")[-1]))
        except ValueError:
            continue
    return analysis_dir / f"opencode_run_{max_idx + 1:03d}.md"


def _append_run_index(project_path: str, record_path: Path, report: dict, runtime_seconds: float) -> None:
    index_path = Path(_analysis_path(project_path)) / "opencode_runs_index.md"
    created = not index_path.exists()
    with index_path.open("a", encoding="utf-8") as f:
        if created:
            f.write("# OpenCode Run Index\n\n")
            f.write("| # | Time | Case | Defect Type | Kept Triplets | Record |\n")
            f.write("|---|---|---|---|---:|---|\n")
        run_no = record_path.stem.split("_")[-1]
        case_id = report.get("case_id", "")
        defect_type = report.get("defect_type", "")
        triplets = (report.get("triplets") or {}).get(defect_type, [])
        f.write(
            f"| {run_no} | {_utc_now()} | {case_id} | {defect_type} | {len(triplets)} | [{record_path.name}]({record_path.name}) |\n"
        )


def _save_run_record(
    *,
    project_path: str,
    action: str,
    payload: dict,
    verify_result: dict,
    export_result: dict,
) -> str:
    record_path = _next_run_record_path(project_path)
    report = export_result.get("report", {})
    runtime_seconds = ((report.get("meta") or {}).get("runtime_seconds"))
    defect_type = report.get("defect_type", "UNKNOWN")
    final_triplets = (report.get("triplets") or {}).get(defect_type, [])

    content = []
    content.append(f"# OpenCode Run Record — {report.get('case_id', os.path.basename(project_path))}")
    content.append("")
    content.append(f"- Recorded at: `{_utc_now()}`")
    content.append(f"- Action: `{action}`")
    content.append(f"- Case: `{report.get('case_id', '')}`")
    content.append(f"- Defect type: `{defect_type}`")
    content.append(f"- Final kept triplets: `{len(final_triplets)}`")
    content.append(f"- Runtime seconds: `{runtime_seconds}`")
    content.append(f"- Report: `{Path(export_result.get('report_path', 'report.yml')).name}`")
    content.append("")
    content.append("## Request Payload")
    content.append("")
    content.append("```json")
    content.append(json.dumps(payload, ensure_ascii=False, indent=2))
    content.append("```")
    content.append("")
    content.append("## Verification Result")
    content.append("")
    content.append("```json")
    content.append(json.dumps(verify_result, ensure_ascii=False, indent=2))
    content.append("```")
    content.append("")
    content.append("## Saved Report Snapshot")
    content.append("")
    content.append("```yaml")
    content.append(Path(export_result["report_path"]).read_text(encoding="utf-8").rstrip())
    content.append("```")
    content.append("")

    record_path.write_text("\n".join(content) + "\n", encoding="utf-8")
    _append_run_index(project_path, record_path, report, float(runtime_seconds or 0))
    return str(record_path)


def main() -> int:
    started_at = time.perf_counter()
    payload = json.load(sys.stdin)
    project_path = payload["project_path"]
    action = payload.get("action", "run_pipeline")

    if action == "run_pipeline":
        analyze_result = _run_analyze(project_path)
        build_result, verify_result = _run_build_and_verify(project_path)
        state = _load_state(project_path)
        state["last_verify_result"] = verify_result
        _save_state(project_path, state)
        json.dump({
            "status": "ok",
            "analyze_result": analyze_result,
            "build_result": build_result,
            "verify_result": verify_result,
        }, sys.stdout, ensure_ascii=False)
        return 0

    if action == "apply_agent_fixes":
        from ice_core.pipeline.repair_models import apply_aggregated_fixes

        repair_result = apply_aggregated_fixes(project_path, payload.get("agent_fixes", []))
        _, verify_result = _run_build_and_verify(project_path)
        state = _load_state(project_path)
        state["last_verify_result"] = verify_result
        _save_state(project_path, state)
        json.dump({
            "status": "ok",
            "patch_result": {"status": "ok", "repair_result": repair_result},
            "verify_result": verify_result,
        }, sys.stdout, ensure_ascii=False)
        return 0

    if action == "apply_false_positive_results":
        state = _load_state(project_path)
        state["false_positive_results"] = payload.get("false_positive_results", [])
        _save_state(project_path, state)
        json.dump({"status": "ok", "saved": len(state["false_positive_results"])}, sys.stdout, ensure_ascii=False)
        return 0

    if action in {"get_final_results", "export_report"}:
        state = _load_state(project_path)
        verify_result = state.get("last_verify_result") or {}
        meta = dict(payload.get("meta") or {})
        export_result = export_report(
            project_path=project_path,
            defects=payload.get("defects") or verify_result.get("defects", []),
            false_positive_results=payload.get("false_positive_results") or state.get("false_positive_results", []),
            defect_type=payload.get("defect_type"),
            case_id=payload.get("case_id"),
            evidence=payload.get("evidence"),
            meta=meta,
            detection_modes=verify_result.get("detection_modes"),
            output_dir=payload.get("output_dir"),
        )
        if action == "get_final_results":
            json.dump({
                "status": "ok",
                "verify_result": verify_result,
                "export_result": export_result,
            }, sys.stdout, ensure_ascii=False)
        else:
            export_result["run_record_path"] = record_path
            json.dump(export_result, sys.stdout, ensure_ascii=False)
        return 0

    json.dump({"status": "error", "errors": [f"unknown action: {action}"]}, sys.stdout, ensure_ascii=False)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
