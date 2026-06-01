import subprocess
from pathlib import Path
from time import time
import json
import os

class UnitTests:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.full_time = 0
        with open("./tests_config.json") as f:
            config = json.load(f)
            self.tests_path = Path(config["tests_path"])
            self.path_to_bin = Path(config["path_to_binary"])

    def run_tests(self):
        files = self.tests_path.rglob("*.volt")
        for test in files:
            start = time();

            res = self.run_process(test)

            t = time() - start;
            self.full_time += t
            format_time = f"{t:.3f}s"

            if not res:
                continue

            test_name = os.path.splitext(test.name)[0]

            if res.returncode != 0:
                self.out("[CRASH]", test_name, res.returncode)
                self.print_output(res)
                self.handle_failed()
                continue

            exp = test.with_suffix(".exp")
            if not exp.exists():
                continue

            exp_text = exp.read_text().strip()

            res_toks = self.tokenize(res.stdout)
            exp_toks = self.tokenize(exp_text)

            if len(res_toks) != len(exp_toks):
                self.out("[FAIL]", test_name, format_time)
                print("Expected:\t", exp_toks)
                print("Got:\t\t", res_toks)
                self.print_output(res)
                self.handle_failed()
                continue

            diffs = self.compare(res_toks, exp_toks)
            if not diffs:
                self.out("[OK]", test_name, format_time)
                self.handle_passed()
                continue

            self.out("[FAIL]", test_name, format_time)
            for i in range(len(res_toks)):
                if i in diffs:
                    print(i + 1, '>', res_toks[i], exp_toks[i])
                    continue

                print(i + 1, res_toks[i], exp_toks[i])

            print(res.stderr)
            self.handle_failed()

    def run_process(self, test):
        try:
            return subprocess.run(
                [self.path_to_bin, str(test)], 
                capture_output=True,
                text=True,
                timeout=5)
        except subprocess.TimeoutExpired as e:
            self.out("[TIME_OUT]", test, "5s")
            self.print_output(e)
            self.handle_failed()
            return None

    def handle_failed(self):
        self.failed += 1

    def handle_passed(self):
        self.passed += 1

    def print_stats(self):
        total = self.passed + self.failed
        print(f"Total:   {total}")
        print(f"Passed:  {self.passed}")
        print(f"Failed:  {self.failed}")
        print(f"Success: {100 * self.passed / total:.0f}%")
        print(f"Time:    {self.full_time:.3f}s")

    def get_typed_token(self, tok):
        if tok.strip() == "true":
            return True

        if tok.strip() == "false":
            return False

        try:
            return int(tok)
        except ValueError:
            pass

        try:
            return float(tok)
        except ValueError:
            pass

        return tok

    def tokenize(self, text):
        toks = text.splitlines()
        typed_toks = []
        for tok in toks:
            if tok == '':
                continue

            typed_toks.append(self.get_typed_token(tok))

        return typed_toks;

    def compare(self, res, exp):
        diffs = []
        for i, (a, b) in enumerate(zip(res, exp)):
            if a != b:
                diffs.append(i)

        return diffs

    def print_output(self, res):
        if res.stdout:
            print(res.stdout)
        if res.stderr:
            print(res.stderr)

    def out(self, state, test, num):
        print(f"{state} {test:<30}{num:>10}")

unit_tests = UnitTests()
unit_tests.run_tests()
print()
unit_tests.print_stats()