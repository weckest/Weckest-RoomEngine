import subprocess
import sys
import platform
from pathlib import Path

PROJECT_ROOT = Path(__file__).resolve().parent.parent

if platform.system() == "Windows":
    PROTO_DIR = PROJECT_ROOT.parent / "Protocols" / "Proto"
elif platform.system() == "Linux":
    PROTO_DIR = PROJECT_ROOT.parent / "Weckest-Protocols" / "Proto"
else:
    raise RuntimeError(
        f"Unsupported operating system: {platform.system()}"
    )

nanopb_generator = (
        PROJECT_ROOT
        / "tools"
        / "nanopb-0.4.9.2-windows-x86"
        / "generator"
        / "nanopb_generator.py"
)

output_dir = PROJECT_ROOT / "src" / "generated"

PROTO_FILES = [
    "hardware.proto",
]

print("======================================")
print("Generating Protocol Buffer files")
print("======================================")

print(f"Proto directory: {PROTO_DIR}")
print(f"Generator:       {nanopb_generator}")
print(f"Output:          {output_dir}")

output_dir.mkdir(parents=True, exist_ok=True)

for proto_name in PROTO_FILES:
    proto_file = PROTO_DIR / proto_name

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
            str(PROTO_DIR),
            "-D",
            str(output_dir),
            str(proto_file)
        ],
        check=True
    )

print("nanopb generation complete.")
