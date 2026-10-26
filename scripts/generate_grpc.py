import subprocess
import sys
from pathlib import Path

project_dir = Path(__file__).resolve().parent.parent

proto_dir = project_dir.parent / "Protocols" / "Proto"

output_dir = project_dir / "src" / "generated" / "grpc"

PROTO_FILES = [
    "render.proto",
]

# Change these paths to wherever protoc and grpc_cpp_plugin are installed.
PROTOC = Path(
    r"C:\path\to\protoc.exe"
)

GRPC_CPP_PLUGIN = Path(
    r"C:\path\to\grpc_cpp_plugin.exe"
)

print("======================================")
print("Generating gRPC Protocol Buffer files")
print("======================================")

print(f"Proto directory: {proto_dir}")
print(f"protoc:          {PROTOC}")
print(f"gRPC plugin:     {GRPC_CPP_PLUGIN}")
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
            str(PROTOC),
            f"--proto_path={proto_dir}",
            f"--cpp_out={output_dir}",
            f"--grpc_out={output_dir}",
            f"--plugin=protoc-gen-grpc={GRPC_CPP_PLUGIN}",
            str(proto_file),
        ],
        check=True
    )

print("gRPC generation complete.")