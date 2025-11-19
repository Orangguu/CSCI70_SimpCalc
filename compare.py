import os

def compare_files(test_file_path, correct_file_path):
    """Compare two text files line by line and print differences."""
    try:
        with open(test_file_path, 'r') as f1, open(correct_file_path, 'r') as f2:
            test_lines = [line.rstrip() for line in f1]
            correct_lines = [line.rstrip() for line in f2]

        # Show line-by-line diff
        differences = []
        max_len = max(len(test_lines), len(correct_lines))

        for i in range(max_len):
            test_line = test_lines[i] if i < len(test_lines) else "<no line>"
            correct_line = correct_lines[i] if i < len(correct_lines) else "<no line>"

            if test_line != correct_line:
                differences.append((i + 1, test_line, correct_line))

        if not differences:
            print(f"✅ {os.path.basename(test_file_path)} matches perfectly.")
            return True
        else:
            print(f"\n❌ Differences in {os.path.basename(test_file_path)}:")
            for line_num, test_line, correct_line in differences:
                print(f"  Line {line_num}:")
                print(f"    test   → {test_line}")
                print(f"    correct→ {correct_line}")
            return False

    except FileNotFoundError:
        print(f"⚠️ File not found: {test_file_path} or {correct_file_path}")
        return False
    except Exception as e:
        print(f"⚠️ Error comparing {test_file_path} and {correct_file_path}: {e}")
        return False


def compare_directories(test_dir, correct_dir):
    """Compare all matching files in two directories."""
    all_results = {}

    for file_name in os.listdir(test_dir):
        test_path = os.path.join(test_dir, file_name)
        correct_path = os.path.join(correct_dir, file_name)

        if os.path.isfile(test_path):
            if os.path.exists(correct_path):
                result = compare_files(test_path, correct_path)
                all_results[file_name] = "✅ MATCH" if result else "❌ DIFFERENT"
            else:
                all_results[file_name] = "⚠️ Missing in correct_dir"

    return all_results


def main():
    test_dir = "output_files"
    correct_dir = "samples-2"

    results = compare_directories(test_dir, correct_dir)

    print("\n📋 Summary:")
    for file, status in results.items():
        print(f"{file:30} → {status}")


if __name__ == "__main__":
    main()
