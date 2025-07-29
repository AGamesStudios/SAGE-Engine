import os


def test_no_json_yaml_usage():
    bad_exts = [".json", ".yaml", ".yml", ".toml", ".xml"]
    errors = []

    for root, _, files in os.walk("sage_engine"):
        for file in files:
            if any(file.endswith(ext) for ext in bad_exts):
                errors.append(os.path.join(root, file))

    assert not errors, f"\u041d\u0430\u0439\u0434\u0435\u043d\u044b \u0437\u0430\u043f\u0440\u0435\u0449\u0451\u043d\u043d\u044b\u0435 \u0444\u043e\u0440\u043c\u0430\u0442\u044b: {errors}"
