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

	if res.returncode != 0:
		print("[CRASH]", res.returncode)
		continue

	exp = test.with_suffix(".exp")
	if not exp.exists():
		continue

	exp_text = exp.read_text().strip()

	res_toks = tokenize(res.stdout)
	exp_toks = tokenize(exp_text)

	if len(res_toks) != len(exp_toks):
		print("[FAIL]", test)
		print("Expected:", exp_toks)
		print("\nGot:", res_toks)
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