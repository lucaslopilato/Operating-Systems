# Script for generating test files

curr_line = []
for i in range(2000):
    curr_line.append(str(i))
    curr_line.append(" The quick brown fox jumped over the lazy dog.")
    curr_line.append("\n")
test_string = ''.join(curr_line)
print test_string
