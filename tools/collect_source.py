
from pathlib import Path
import os


# ============================================================
# 配置
# ============================================================

SOURCE_DIRS = [
    "include",
    "src",
]

FILE_EXTENSIONS = {
    ".h",
    ".cpp",
}

OUTPUT_FILE_NAME = "project_source.md"


# ============================================================
# 路径
# ============================================================

# 当前 Python 脚本所在目录
ROOT_DIR = Path(__file__).resolve().parent


def get_desktop_path() -> Path:
    """
    获取 Windows 桌面目录。
    优先普通 Desktop，如果使用 OneDrive Desktop 则自动尝试。
    """

    home = Path.home()

    candidates = [
        home / "Desktop",
        home / "OneDrive" / "Desktop",
        home / "OneDrive" / "桌面",
    ]

    for path in candidates:
        if path.exists():
            return path

    # 实在找不到就创建 ~/Desktop
    desktop = home / "Desktop"
    desktop.mkdir(parents=True, exist_ok=True)
    return desktop


DESKTOP_DIR = get_desktop_path()
OUTPUT_PATH = DESKTOP_DIR / OUTPUT_FILE_NAME


# ============================================================
# 文件收集
# ============================================================

def collect_source_files():
    files = []

    for source_dir_name in SOURCE_DIRS:
        source_dir = ROOT_DIR / source_dir_name

        if not source_dir.exists():
            print(f"[Warning] 找不到目录: {source_dir}")
            continue

        for file_path in source_dir.rglob("*"):
            if (
                file_path.is_file()
                and file_path.suffix.lower() in FILE_EXTENSIONS
            ):
                files.append(file_path)

    # 根据相对路径排序，使输出稳定
    files.sort(
        key=lambda path: str(path.relative_to(ROOT_DIR)).lower()
    )

    return files


# ============================================================
# 文件读取
# ============================================================

def read_text_file(file_path: Path) -> str:
    """
    尝试使用常见编码读取源码。
    """

    encodings = [
        "utf-8",
        "utf-8-sig",
        "gbk",
        "gb18030",
    ]

    for encoding in encodings:
        try:
            return file_path.read_text(encoding=encoding)
        except UnicodeDecodeError:
            pass

    # 最后的兜底方案
    return file_path.read_text(
        encoding="utf-8",
        errors="replace"
    )


# ============================================================
# Markdown 生成
# ============================================================

def generate_markdown(files):
    lines = []

    lines.append("# Project Source Code")
    lines.append("")
    lines.append(f"Root: `{ROOT_DIR}`")
    lines.append("")
    lines.append(f"Total files: **{len(files)}**")
    lines.append("")

    # --------------------------------------------------------
    # 文件目录
    # --------------------------------------------------------

    lines.append("## File List")
    lines.append("")

    for file_path in files:
        relative_path = file_path.relative_to(ROOT_DIR)
        lines.append(f"- `{relative_path.as_posix()}`")

    lines.append("")
    lines.append("---")
    lines.append("")

    # --------------------------------------------------------
    # 文件正文
    # --------------------------------------------------------

    for index, file_path in enumerate(files, start=1):
        relative_path = file_path.relative_to(ROOT_DIR)
        relative_path_str = relative_path.as_posix()

        print(
            f"[{index}/{len(files)}] "
            f"{relative_path_str}"
        )

        content = read_text_file(file_path)

        lines.append(f"## `{relative_path_str}`")
        lines.append("")

        # cpp fenced code 对 .h / .cpp 都可以正常高亮
        lines.append("```cpp")
        lines.append(content.rstrip())
        lines.append("```")
        lines.append("")
        lines.append("---")
        lines.append("")

    return "\n".join(lines)


# ============================================================
# Main
# ============================================================

def main():
    print("=" * 60)
    print("Collect C++ Source Files")
    print("=" * 60)

    print(f"Project Root : {ROOT_DIR}")
    print(f"Output       : {OUTPUT_PATH}")
    print()

    files = collect_source_files()

    if not files:
        print("没有找到任何 .h 或 .cpp 文件。")
        return

    print(f"找到 {len(files)} 个文件。")
    print()

    markdown = generate_markdown(files)

    OUTPUT_PATH.write_text(
        markdown,
        encoding="utf-8"
    )

    print()
    print("=" * 60)
    print("完成!")
    print(f"输出文件: {OUTPUT_PATH}")
    print("=" * 60)


if __name__ == "__main__":
    main()

