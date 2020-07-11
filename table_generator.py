
class SLR_item:
	def __init__(self, head, body, index, production_number):
		self.head = head
		self.body = body
		self.length = len(body)
		self.index = index
		self.production_number = production_number
	def __str__(self):
		return f"SLR: '{self.head} -> {self.body}'"
	def __repr__(self):
		return self.__str__()
	def __getitem__(self, index):
		return self.body[index]
	def next_symbol(self):
		index = self.index
		if index < self.length:
			return self.body[index]
		else:
			return None
	def next_item(self):
		index = self.index
		if index < self.length:
			return self.body, index+1
		return None
	def create_LR_items(self, lookaheads):
		LR_items = set()
		head = self.head
		body = self.body
		index = self.index
		production_number = self.production_number
		for lookahead in lookaheads:
			LR_items.add(LR_item(head, body, index, lookahead,
								production_number, lookahead))
		return LR_items

class LR_item:
	def __init__(self, head, body, index, production_number, lookahead):
		self.head = head
		self.body = body
		self.length = len(body)
		self.index = index
		self.lookahead = lookahead
		self.production_number = production_number
	def __str__(self):
		return f"'{self.head} -> {self.body}'"
	def __repr__(self):
		return self.__str__()
	def __getitem__(self, index):
		return self.body[index]
	def next_symbol(self):
		index = self.index
		if index < self.length:
			return self.body[index]
		else:
			return None
	def next_item(self):
		index = self.index
		if index < self.length:
			return body, index+1
		return None
	def next_lookahead(first, nullable):
		next_symbol = self.next_symbol()
		if self.next_symbol == None:
			return {self.lookahead}
		else:
			lookaheads = set()
			for symbol in first[next_symbol]:
				lookaheads.add(symbol)
			if nullable[next_symbol]:
				lookaheads.add(self.lookahead)
			return lookaheads

def first_and_follow(productions, nonterminals, terminals):
	first = dict()
	follow = dict()
	nullable = dict()
	for nonterminal in nonterminals:
		first[nonterminal] = set()
		follow[nonterminal] = set()
		nullable[nonterminal] = False
	for terminal in terminals:
		first[terminal] = {terminal}
		nullable[terminal] = False
		follow[terminal] = set()
	follow['start'] = {'$'}
	last_hash = 0
	curr_hash = ffnhash(first, follow, nullable)
	while curr_hash != last_hash:
		for symbol, products in productions.items():
			for production in products:
				body = production.split()
				l = len(body)
				i = 0
				while i < l and nullable[body[i]]:
					i += 1
				if i == l:
					nullable[symbol] = True
					continue
				first[symbol] |= first[body[i]]
				i = l-1
				while i >= 0 and nullable[body[i]]:
					i =- 1
				follow[body[i]] |= follow[symbol]
				for i, b in enumerate(body):
					while i < l and nullable[body[i]]:
						i += 1
					if i < l-1:
						follow[b] |= first[body[i+1]]
		last_hash = curr_hash
		curr_hash = ffnhash(first, follow, nullable)

	return first, follow, nullable

def ffnhash(first, follow, nullable):
	hash = 0x811c9dc5
	for s in first.values():
		hash = (hash ^ len(s))*0x01000193
	hash %= 10000000
	for s in follow.values():
		hash = (hash ^ len(s))*0x01000193
	hash %= 10000000
	for s in nullable.values():
		hash = (hash ^ s)*0x01000193
	return hash % 10000000


def generate_SLR_items():
		items = dict()
		nonterminals = set()
		terminals = {'$'}
		productions = dict()
		rules = list()
		with open("grammartest", "r", encoding="utf-8") as f:
			prod_num = 0
			for s in f:
				if s == '\n':
					continue
				prod_head = ""
				i = 0
				while (s[i] != ' ' and s[i] != '-'):
					prod_head += s[i]
					i += 1
				while (s[i] == ' ' or s[i] == '-' or s[i] == '>'):
						i += 1
				s_i = i
				if (items.get(prod_head) == None):
					items[prod_head] = set()
					productions[prod_head] = set()

				or_productions = [or_production.strip()
								  for or_production in s[s_i:-1].split('|')]


				for or_production in or_productions:
					rules.append((prod_head, or_production))
					symbols = or_production.split(' ')
					#Add the first item which always begins with a '.'
					item = SLR_item(prod_head, symbols, 0, prod_num)
					items[prod_head].add(item)
					for i, symbol in enumerate(symbols):
						if (symbol[0] == "'" and symbol[-1] == "'"):
							terminals.add(symbol)
						else:
							nonterminals.add(symbol)
						item = SLR_item(prod_head,
										symbols, i+1,
										prod_num)
						items[prod_head].add(item)
					prod_num += 1
					productions[prod_head].add(or_production)
				print(prod_head, items[prod_head])
		return rules, items, productions, nonterminals, terminals

def generate_SLR_parsing_table(items, nonterminals, terminals, follow):
	table = list()
	collection = list()
	new_sets = list()
	new_sets.append(frozenset(SLR_CLOSURE({SLR_item("start", ['S'], 0, 0)}, items)))
	j = 0
	while True:
		tmp_sets = list()
		i = len(collection)+len(new_sets)
		for item_set in new_sets:
			tmp_row = dict()
			tmp_row[0] = item_set
			j += 1
			for terminal in terminals:
				tmp = SLR_GOTO(item_set, terminal, items)

				if tmp is not None:
					tmp = frozenset(tmp)
					if tmp in collection:
						tmp_row[terminal] = (0, collection.index(tmp))
					else:
						if tmp in new_sets:
							tmp_row[terminal] = (0, new_sets.index(tmp)+len(collection))

						else:
							tmp_row[terminal] = (0, i)
							tmp_sets.append(tmp)
							i += 1
				else:
					for item in item_set:
						if item.index == item.length:
							if terminal in follow[item.head]:
								tmp_row[terminal] = (1, item.production_number)


			for nonterminal in nonterminals:
				tmp = SLR_GOTO(item_set, nonterminal, items)
				if tmp is not None:
					tmp = frozenset(tmp)
					if tmp in collection:
						tmp_row[nonterminal] = (0, collection.index(tmp))
					else:
						if tmp in new_sets:
							tmp_row[nonterminal] = (0, new_sets.index(tmp)+len(collection))
						else:
							tmp_row[nonterminal] = (0, i)
							tmp_sets.append(tmp)
							i += 1

				else:
					for item in item_set:
						if item.index == item.length:
							if nonterminal in follow[item.head]:
								tmp_row[terminal] = (1, item.production_number)
			table.append(tmp_row)
		if tmp_sets == list():
			break
		for new_set in new_sets:
			collection.append(new_set)
		new_sets = list(tmp_sets)

	return collection ,table

def generate_SLR_collection(items, nonterminals, terminals, follow):
	collection = list()
	new_sets = list()
	new_sets.append(frozenset(SLR_CLOSURE({SLR_item("start", ['S'], 0, 0)}, items)))
	while True:
		tmp_sets = list()
		for item_set in new_sets:
			for terminal in terminals:
				tmp = SLR_GOTO(item_set, terminal, items)
				if tmp is not None:
					tmp = frozenset(tmp)
					if not tmp in collection and not tmp in new_sets:
						tmp_sets.append(tmp)

			for nonterminal in nonterminals:
				tmp = SLR_GOTO(item_set, nonterminal, items)
				if tmp is not None:
					tmp = frozenset(tmp)
					if not tmp in collection and not tmp in new_sets:
							tmp_sets.append(tmp)

		if tmp_sets == list():
			break
		for new_set in new_sets:
			collection.append(new_set)
		new_sets = list(tmp_sets)
	return collection

def SLR_CLOSURE(current_set, items):
	# The CLOSURE function for SLR explained in the dragon book
	closure = list(current_set)
	added = 1
	while (added):
		added = 0
		tmp_set = list()
		for item in closure:
			next_symbol = item.next_symbol()
			if (items.get(next_symbol) != None):
				for item in items[next_symbol]:
					if item.index == 0 and item not in closure:
						tmp_set.append(item)
						added = 1
		for item in tmp_set:
			if item not in closure:
				closure.append(item)

	return closure

def SLR_GOTO(current_set, symbol, items):
	# The GOTO function for SLR explained in the dragon book
	next_set = list()
	print(current_set)
	for item in current_set:
		next_symbol = item.next_symbol()
		print(f"Goto {symbol} on item {item} : {next_symbol}")
		if symbol == next_symbol:
			next_item = item.next_item()
			if next_item != None:
				for item in items[item.head]:
					print(next_item)
					if item.body == next_item[0] and item.index == next_item[1]:
						if next_item not in next_set:
							next_set.append(next_item)
						break
	print(next_set)
	if next_set:
		return SLR_CLOSURE(next_set, items)
	return None

def SLR_kernel(collection):
	kernel_collection = list()
	kernel_set = set()
	for item in collection[0]:
		if item.head == 'start':
			kernel_set.add(item)
		elif item.index > 0:
			kernel_set.add(item)
	kernel_collection.append(kernel_set)
	for item_set in collection[1:]:
		kernel_set = set()
		for item in item_set:
			if item.index > 0:
				kernel_set.add(item)
		kernel_collection.append(kernel_set)
	return kernel_collection

def LR_parsing_algorithm(parsing_table, reduction_rules, input):
	reduction_rules = [(r[0], r[1].split()) for r in reduction_rules]
	stack = list()
	stack.append(0)

	input = "'" + input.replace(" ", "' '") + "'"
	input = input.split()
	i = 0
	while 1:
		a = input[i] if i < len(input) else '$'
		action = parsing_table[stack[-1]].get(a)
		if action == None:
			print(f"error in state {stack[-1]} on symbol {a}")
			break
		elif action[0] == 0:
			stack.append(action[1])
			i += 1
		else:
			if action[1] == 0:
				print("parsing done")
				break
			else:
				r = reduction_rules[action[1]]
				for _ in range(len(r[1])):
					stack.pop()
				stack.append(parsing_table[stack[-1]].get(r[0])[1])
				print(r[0] + " -> " + " ".join(r[1]))
				print("stack:", stack, "left to parse:", input[i:])

rules, items, productions, nonterminals, terminals = generate_SLR_items()
print("rules", rules)
print("productions", productions)
print("nonterminals", nonterminals)
print("terminals", terminals)
nonterminals.add('start')
first, follow, nullable = first_and_follow(productions, nonterminals, terminals)
collection = generate_SLR_collection(items, nonterminals, terminals, follow)
print("SLR collection!")
for s in collection:
	print(s)

c, t = generate_SLR_parsing_table(items, nonterminals, terminals, follow)

print("ITEM SET")
for i, r in enumerate(t):
	print(f"I{i}")
	for f in r[0]:
		print(f)
	print()
for i, rule in enumerate(rules):
	print(f"({i}) {rule[0]} -> {rule[1]}")
n_terminals = len(terminals)
n_nonterminals = len(nonterminals)
print("\u250C"+("\u2500"*7+"\u252C")+("\u2500")*(n_terminals*8-1)+"\u252C"+"\u2500"*(n_nonterminals*8-1)+"\u2510")
print("\u2502 STATE \u2502 ", end="\t")
print("\t"*(n_terminals-n_terminals//2-2)+"      "\
	"ACTION"+"  "+"\t"*(n_terminals//2), end = "")
print("\u2502"+"\t "*(n_nonterminals-n_nonterminals//2-1)+\
	"      GOTO"+"\t"*(n_nonterminals//2)+"\u2502")
print("\u2502", end="\t")
print("\u251C"+7*"\u2500"+("\u252C"+7*"\u2500")*(n_terminals-1)+"\u253C"+(7*"\u2500"+"\u252C")*(n_nonterminals-1)+7*"\u2500"+"\u2524")
print("\u2502", end="\t")

for terminal in terminals:
	print(f"\u2502 {terminal}", end="\t")
for nonterminal in nonterminals:
	print(f"\u2502 {nonterminal}", end="\t")
print("\u2502")
print("\u251C"+("\u2500"*7+"\u253C")*(n_terminals+n_nonterminals)+"\u2500"*7+"\u2524")
for i, r in enumerate(t):
	print(f"\u2502 {i}", end="\t")
	for terminal in terminals:
		if r.get(terminal):
			x = r[terminal]
			if x[0] == 0:
				print(f"\u2502 s{x[1]}", end="\t")
			else:
				print(f"\u2502 r{x[1]}", end="\t")
		else:
			print("\u2502", end="\t")
	for nonterminal in nonterminals:
		if r.get(nonterminal):
			x = r[nonterminal]
			if x[0] == 0:
				print(f"\u2502 s{x[1]}", end="\t")
			else:
				print(f"\u2502 r{x[1]}", end="\t")
		else:
			print("\u2502", end="\t")
	print("\u2502")
print("\u2514"+("\u2500"*7+"\u2534")*(n_terminals+n_nonterminals)+"\u2500"*7+"\u2518")
while True:
	i = input()
	LR_parsing_algorithm( t, rules, i)
