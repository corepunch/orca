import sys, struct, os

input = sys.argv[1]
output = sys.argv[2]
charset = len(sys.argv) > 3 and sys.argv[3] or "charset"

def writefile(fin, fout):
	i = 0
	size = 0
	bytes = fin.read(1)
	while bytes:
		value = struct.unpack('B', bytes)
		fout.write("0x%02x," % value)
		i = i + 1
		size = size + 1
		if i == 16:
			fout.write("\n  ")
			i = 0
		bytes = fin.read(1)
	return size

def replace_sym(var):
	var = var.replace("-", "_")
	return "file_" + var

if os.path.isdir(input):
	with open(output, "w") as fout:
		fout.write("#include <string.h>\n\n")
		for filename in os.listdir(input):
			f = os.path.join(input, filename)
			var = filename.split(".")[0]
			if not var:
				continue
			with open(f, "rb") as fin:
				fout.write("static const unsigned char %s[] = {\n  " % replace_sym(var))
				writefile(fin, fout)
				fout.write("\n};\n")
		fout.write("int Get%s(char const *name, const void **buf) {\n" % os.path.basename(input))
		for filename in os.listdir(input):
			var = filename.split(".")[0]
			if not var:
				continue
			fout.write("  if (!strcmp(name, \"%s\")) { *buf = %s; return sizeof(%s); } \n" % (var, replace_sym(var), replace_sym(var)))
		fout.write("  return 0;\n")
		fout.write("}\n")
else:
	with open(input, "rb") as fin, open(output, "w") as fout:
		fout.write("const unsigned char %s[] = {\n  " % charset)
		size = writefile(fin, fout)
		fout.write("\n};\n")
		fout.write("const int %s_size = %d;\n" % (charset, size))
