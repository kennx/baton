#!/usr/bin/env python3
import sys
import os

def parse_flipper_hex(hex_str):
    # Flipper stores values as Little Endian byte strings: "04 00 00 00" -> 0x00000004 -> "0x04"
    parts = hex_str.strip().split()
    if not parts:
        return "0x00"
    
    # Reverse to make it Big Endian for integer parsing
    parts.reverse()
    hex_val = "".join(parts)
    # Strip leading zeros
    hex_val = hex_val.lstrip('0')
    if not hex_val:
        hex_val = "0"
    return f"0x{hex_val.upper()}"

def convert_ir_file(filepath):
    if not os.path.exists(filepath):
        print(f"File not found: {filepath}")
        return

    brand = os.path.basename(os.path.dirname(filepath))
    filename = os.path.basename(filepath).replace(".ir", "")

    protocol = "UNKNOWN"
    address = "0x00"
    commands = []

    current_cmd_name = ""
    current_type = ""

    with open(filepath, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or line.startswith('Filetype:') or line.startswith('Version:'):
                continue
            
            if line.startswith('name:'):
                current_cmd_name = line.split(':', 1)[1].strip()
            elif line.startswith('type:'):
                current_type = line.split(':', 1)[1].strip()
            elif line.startswith('protocol:'):
                protocol = line.split(':', 1)[1].strip()
            elif line.startswith('address:'):
                addr_str = line.split(':', 1)[1].strip()
                address = parse_flipper_hex(addr_str)
            elif line.startswith('command:'):
                if current_type == "parsed":
                    cmd_str = line.split(':', 1)[1].strip()
                    hex_cmd = parse_flipper_hex(cmd_str)
                    commands.append((current_cmd_name, hex_cmd))
                current_cmd_name = ""

    if not commands:
        print(f"// No 'parsed' commands found in {filename}.ir")
        return

    # Generate C++ code
    print(f"  // Auto-generated from {brand}/{filename}.ir")
    print(f"  profiles.push_back({{")
    print(f"    \"{brand} {filename}\",")
    print(f"    \"{protocol}\",")
    print(f"    \"{address}\",")
    print(f"    {{")
    for idx, (name, cmd) in enumerate(commands):
        comma = "," if idx < len(commands) - 1 else ""
        print(f"      {{\"{name}\", \"{cmd}\"}}{comma}")
    print(f"    }}")
    print(f"  }});")
    print()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python import_flipper.py <path_to_flipper_ir_file>")
        sys.exit(1)
    
    target = sys.argv[1]
    if os.path.isdir(target):
        for root, dirs, files in os.walk(target):
            for file in files:
                if file.endswith(".ir"):
                    convert_ir_file(os.path.join(root, file))
    else:
        convert_ir_file(target)
