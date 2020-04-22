import os

core = os.environ["CORE"]

f = open(core + "/command/rendercommands.hpp", 'r')

string = ""
for line in f:
    if line[:5] == "class":
        tokens = line.split()
        name = tokens[1]
        if name[-1] == ':':
            name = name[:-1]
        func = lambda s: s[:1].lower() + s[1:] if s else ''
        lower_name = func(name)
        l = "CommandPool<command::" + name + "> " + lower_name + ";\n"
        string += l

print string

f.close()
