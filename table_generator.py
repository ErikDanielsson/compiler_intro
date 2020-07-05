class SLR_item:
	def __init__(self, head, body):
		self.head = head
		self.body = body
	def __str__(self):
		return f"'{self.head} -> {self.body}'"
	def __repr__(self):
		return f"{self.body}"
	def __getitem__(self, index):
		return self.body[index]
	def next_symbol(self):
		index = self.body.index('.')
		if (index != len(self.body)-1):
			for i, char in enumerate(self.body[index+1:]):
				if not char.isspace():
					break;
			symbol = ""
			for i, char in enumerate(self.body[index+1+i:]):
				if char.isspace():
					break
				symbol += char
			return symbol
		return ""
	def str_of_next_item(self):
		l = len(self.body)
		index = self.body.index('.')
		to_dot = self.body[:index]
		i = index+1
		while (i < l and self.body[i].isspace()):
			to_dot += self.body[i]
			i+= 1
		while (i < l and not self.body[i].isspace()):
			to_dot += self.body[i]
			i+= 1
		to_dot += '.'
		to_dot += self.body[i:]
		return to_dot


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
		terminals = set()
		productions = dict()
		with open("grammartest", "r", encoding="utf-8") as f:
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
				#Add the first item which always begins with a '.'
				item = SLR_item(prod_head, s[s_i:i] + '.' + s[i:-1])
				items[prod_head].add(item)
				last_i = i
				while (s[i] != "\n"):
					# For each symbols in production, make one item.
					while (s[i] == ' ' or s[i] == '\t'):
						if (s[i] == "\n"):
							break
						i += 1
					last_i = i

					while (s[i] != ' ' and s[i] != '\t'):
						if (s[i] == "\n"):
							break
						i += 1
					symbol = s[last_i:i]
					if symbol:
						if (symbol[0] == "'" and symbol[-1] == "'"):
							terminals.add(symbol)
						else:
							nonterminals.add(symbol)
					body = s[s_i:i] + '.' + s[i:-1]
					item = SLR_item(prod_head, body)
					items[prod_head].add(item)
				productions[prod_head].add(s[s_i:-1])
		return items, productions, nonterminals, terminals

def generate_SLR_sets(items, terminals, nonterminals):

	collection = set()
	new_sets = set()
	new_sets.add(frozenset(CLOSURE({SLR_item("start", ".E")}, items)))
	while True:
		tmp_sets = set()
		for item_set in new_sets:
			for symbol in symbols:
				tmp = GOTO(item_set, symbol, items)
				if tmp is not None:
					tmp = frozenset(tmp)
					if tmp not in collection.union(new_sets):
						tmp_sets.add(frozenset(tmp))
		if tmp_sets == set():
			break
		collection = collection.union(new_sets)
		new_sets = set(tmp_sets)
	return collection



def generate_SLR_parsing_table(items, nonterminals, terminals, follow):
	table = list()
	collection = list()
	new_sets = list()
	new_sets.append(frozenset(CLOSURE({SLR_item("start", ".E")}, items)))
	while True:
		tmp_sets = list()
		i = len(collection)+len(new_sets)
		for item_set in new_sets:
			tmp_row = dict()
			tmp_row[0] = item_set
			for terminal in terminals:
				tmp = GOTO(item_set, terminal, items)

				if tmp is not None:
					tmp = frozenset(tmp)
					if tmp in collection:
						tmp_row[terminal] = collection.index(tmp)
					else:
						if tmp in new_sets:
							tmp_row[terminal] = new_sets.index(tmp)+len(collection)
						else:
							tmp_row[terminal] = i
							tmp_sets.append(tmp)
							i += 1
				for item in item_set:
					if item.body[-1] == '.':
						if terminal in follow[item.head]:
							tmp_row[terminal] = item.head


			for nonterminal in nonterminals:
				tmp = GOTO(item_set, nonterminal, items)
				if tmp is not None:
					tmp = frozenset(tmp)
					if tmp in collection:
						tmp_row[nonterminal] = collection.index(tmp)
					else:
						if tmp in new_sets:
							tmp_row[nonterminal] = new_sets.index(tmp)+len(collection)
						else:
							tmp_row[terminal] = i
							tmp_sets.append(tmp)
							i += 1
					for item in item_set:
						if item.body[-1] == '.':
							if nonterminal in follow[item.head]:
								tmp_row[terminal] = item.head
			print()
			for k, s in tmp_row.items():
				print(k, s)
			print()
			table.append(tmp_row)
		if tmp_sets == list():
			break
		for new_set in new_sets:
			collection.append(new_set)
		new_sets = list(tmp_sets)

	return collection ,table



def CLOSURE(current_set, items):
	# The CLOSURE function for SLR explained in the dragon book
	closure = set(current_set)
	added = 1
	while (added):
		added = 0
		tmp_set = set()
		for item in closure:
			next_symbol = item.next_symbol()
			if (items.get(next_symbol) != None):
				for item in items[next_symbol]:
					if item[0] == '.' and item not in closure:
						tmp_set.add(item)
						added = 1
		closure = closure.union(tmp_set)

	return closure

def GOTO(current_set, symbol, items):
	# The GOTO function for SLR explained in the dragon book
	next_set = set()
	for item in current_set:
		next_symbol = item.next_symbol()
		if symbol == next_symbol:
			next_body = item.str_of_next_item()
			for next_item in items[item.head]:
				if next_item.body == next_body:
					next_set.add(next_item)
					break
	if next_set:
		return CLOSURE(next_set, items)
	return None

items, productions, nonterminals, terminals = generate_SLR_items()
print("productions:", productions)
print("item:", items)
print("nonterminals", nonterminals)
nonterminals.add('start')
print("terminals", terminals)
first, follow, nullable = first_and_follow(productions, nonterminals, terminals)
print("first: ")
for key, symbols in first.items():
	print(f"{key}: {symbols}")
print("follow: ")

for key, symbols in follow.items():
	print(f"{key}: {symbols}")

#for key, symbols in compute_follow(production_bodies, nonterminals, terminals).items():
	#print(f"{key}: {symbols}")
#collection = generate_SLR_sets(items, nonterminals, terminals)
#for s in collection:
#        print(s)

c, t = generate_SLR_parsing_table(items, nonterminals, terminals, follow)
for s in c:
	print(c)
	print()
print("table")
for i, r in enumerate(t):
	for f in r[0]:
		print(f, end = " ")
	print()
	print("action")

	for key, value in r.items():
		if key == 0:
			continue
		print(key, value)
	print()
