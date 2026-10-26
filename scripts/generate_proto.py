import subprocess
import sys
from pathlib import Path

project_dir = Path(__file__).resolve().parent.parent

proto_dir = project_dir.parent / "Protocols" / "Proto"

nanopb_generator = (
        project_dir
        / "tools"
        / "nanopb-0.4.9.2-windows-x86"
        / "generator"
        / "nanopb_generator.py"
)

output_dir = project_dir / "src" / "generated"

PROTO_FILES = [
    "hardware.proto",
]

print("======================================")
print("Generating Protocol Buffer files")
print("======================================")

print(f"Proto directory: {proto_dir}")
print(f"Generator:       {nanopb_generator}")
print(f"Output:          {output_dir}")

output_dir.mkdir(parents=True, exist_ok=True)

for proto_name in PROTO_FILES:
    proto_file = proto_dir / proto_name

    if not proto_file.exists():
        raise FileNotFoundError(
            f"Proto file does not exist: {proto_file}"
        )

    print(f"Generating: {proto_file.name}")

    subprocess.run(
        [
            sys.executable,
            str(nanopb_generator),
            "-I",
            str(proto_dir),
            "-D",
            str(output_dir),
            str(proto_file)
        ],
        check=True
    )

print("nanopb generation complete.")