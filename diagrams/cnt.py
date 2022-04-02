st = "object ::= \"{\" kvpair kvpair2 \"}\"\nkvpair ::= string \":\" value\nkvpair2 ::= \",\" kvpair kvpair2 | ε\nstring ::= \"\"\" charsolo charloop \"\"\"\nvalue ::= array | object | number | string | \"true\" | \"false\" | \"null\"\narray ::= \"[\" value value2 \"]\"\nvalue2 ::= \",\" value value2 | ε\nnumber ::= \"-\" \"0\" fraction | \"-\" \"0\" fraction exponent | \"-\" \"0\" exponent | \"-\" digit digitloop | \"-\" digit digitloop fraction | \"-\"  digit digitloop fraction exponent | \"-\"  digit digitloop exponent | \"0\" | digit digitloop | digit digitloop fraction | digit digitloop fraction exponent | digit digitloop exponent\nfraction ::= \".\" digitwithzero digitloop\ndigit ::= \"1\" | \"2\" | \"3\" | \"4\" | \"5\"\ndigitwithzero ::= \"0\" | digit\ndigitloop ::= digitwithzero digitloop | ε\nexponent ::= \"e\" digitwithzero digitloop | \"e\" \"-\" digitwithzero digitloop | \"e\" \"+\" digitwithzero digitloop | \"E\" digitwithzero digitloop | \"E\" \"-\" digitwithzero digitloop | \"E\" \"+\" digitwithzero digitloop\ncharsolo ::= \"a\" | \"b\" | \"c\" | \"x\" | \"y\" | \"z\"\ncharloop ::= charsolo charloop | ε";

symbs = set(st.split()) - {"::=", "|"}

print(symbs)
print(len([s for s in symbs if "\"" in s]))
print(len(symbs))
