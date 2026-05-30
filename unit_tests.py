import subprocess
from pathlib import Path

def get_typed_token(tok):
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

def tokenize(text):
	toks = text.splitlines()
	typed_toks = []
	for tok in toks:
		if tok == '':
			continue

		typed_toks.append(get_typed_token(tok));

	return typed_toks;

def compare(res, exp):
	diffs = []
	for i in range(len(res)):
		if res[i] != exp[i]:
			diffs.append(i)

	return diffs

def print_output(res):
	if res.stdout:
		print(res.stdout)
	if res.stderr:
		print(res.stderr)

tests_path = Path("Tests")

for test in tests_path.rglob("*.volt"):
	try:
		res = subprocess.run(
			["cmake-build-release/CVolt", str(test)], 
			capture_output=True,
	        text=True,
	        timeout=5)
	except subprocess.TimeoutExpired:
		print("[TIME_OUT]", test)
		print(res.stderr)

	if res.returncode != 0:
		print("[CRASH]", res.returncode)
		print_output(res)
		continue

	exp = test.with_suffix(".exp")
	if not exp.exists():
		continue

	exp_text = exp.read_text().strip()

	res_toks = tokenize(res.stdout)
	exp_toks = tokenize(exp_text)

	if len(res_toks) != len(exp_toks):
		print("[FAIL]", test)
		print("Expected:\t", exp_toks)
		print("Got:\t\t", res_toks)

		print_output(res)
		continue

	diffs = compare(res_toks, exp_toks)
	if diffs == []:
		print("[OK]", test);
		continue

	print("[FAIL]", test)
	for i in range(len(res_toks)):
		if i in diffs:
			print(i + 1, '>', res_toks[i], exp_toks[i])
			continue

		print(i + 1, res_toks[i], exp_toks[i])

	print(res.stderr)