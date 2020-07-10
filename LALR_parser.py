"""
Pipeline for LALR parsing table construction:
construct LALR kernels
	construct SLR kernels and save GOTO for each symbol since it will not depend
	on lookahead
		construct SLR collection and transition table
			algorithms:
				construct SLR items
				table construction
				GOTO for SLR
				CLOSURE for SLR
		remove non-kernel items
			loop over items in all sets
	compute which lookahead types for kernels
		Apply algorithm 4.62 on each kernel set in SLR collection
	create table for lookahead propagation
	compute lookaheads
generate LALR parsing table:
	compute LR CLOSURE on kernels to generate full sets
	generate table according to algorithm 4.56.


"""
import sys


class item:
	def __init__(self, head, body, index, prod_num):
		self.head = head
		self.body = body
		self.length = len(body)
		self.index = index
		self.prod_num = prod_num
	def __len__(self):
		return self.length
	def __eq__(self, item):
		if type(item) == tuple:
			return self.head == item[0] and \
			       self.body == item[1] and \
				   self.index == item[2]
		return self.head == item.head and \
		       self.body == item.body and \
			   self.index == item.index
	def next_symbol(self):
		index = self.index
		if index < self.length:
			return self.body[index]
		return None


class SLR_item(item):
	def __init__(self, head, body, index, prod_num):
		super().__init__(head, body, index, prod_num)
	def get_LR_item(self):
		return LR_item(self.head, self.body, self.index, self.prod_num)
	def __str__(self):
		s1 = " ".join(self.body[:self.index])
		s2 = " ".join(self.body[self.index:])
		return (f"[{self.head} -> {s1}. "
		       f"{s2}]")
	def __repr__(self):
		return self.__str__()
	def next_item(self):
		index = self.index
		if index < self.length:
			return SLR_item(self.head, self.body, index+1, self.prod_num)
		return None
	def __hash__(self):
		return hash((self.index, self.prod_num))

class LR_item(item):
	def __init__(self, head, body, index, prod_num):
		super().__init__(head, body, index, prod_num)
		self.lookaheads = set()
	def add(self, lookahead):
		self.lookaheads.add(lookahead)
	def union(self, other_set):
		self.lookaheads.update(other_set)
	def __len__(self):
		return len(self.lookaheads)
	def __contains__(self, lookahead):
		return lookahead in self.lookaheads
	def __iter__(self):
		return iter(self.lookaheads)
	def __str__(self):
		s1 = " ".join(self.body[:self.index])
		s2 = " ".join(self.body[self.index:])
		s3 = "/".join(self.lookaheads)
		return (f"[{self.head} -> {s1}. "
		       f"{s2}, {s3}]")
	def __repr__(self):
		return self.__str__()
	def join(self, item):
		if self.head == item.head and self.body == item.body and \
			   self.index == item.index:
			self.union(item.lookaheads)
			return True
		return False
	def next_next_symbol(self):
		index = self.index
		if index < self.length-1:
			return self.body[index+1]
		return None
	def next_item(self):
		index = self.index
		if index < self.length:
			return (self.head, self.body, index+1)
		return None
	def __hash__(self):
		return hash((self.index, self.prod_num, "".join(self.lookaheads)))

class SLR_item_set:
	def __init__(self, *args):
		if args:
			arg = args[0]
			if type(arg) == SLR_item_set:
				self.set = list(arg.set)
			elif type(arg) == SLR_item:
				self.set = [arg]
			else:
				print("error: Mismatched types")
				quit()
		else:
			self.set = list()
	def add(self, slr_item):
		if slr_item not in self.set:
			self.set.append(slr_item)
	def union(self, item_set):
		for item in item_set:
			self.add(item)
	def __getitem__(self, index):
		return self.set[index]
	def __len__(self):
		return len(self.set)
	def __eq__(self, item_set):
		return self.set == item_set.set
	def __str__(self):
		return '\t'+'\n\t'.join([str(item) for item in self.set])
	def __repr__(self):
		return self.__str__()
	def __hash__(self):
		return hash(tuple(self.set))
	def get_LR_item_set(self):
		set = LR_item_set()
		for item in self.set:
			set.add(item.get_LR_item())
		return set

class LR_item_set:
	def __init__(self, *args):
		if args:
			arg = args[0]
			if type(arg) == LR_item_set:
				self.set = list(arg.set)
			elif type(arg) == LR_item:
				self.set = [arg]
			else:
				print("error: Mismatched types")
				quit()
		else:
			self.set = list()
	def add(self, lr_item):
		for item in self.set:
			if item.join(lr_item):
				return
		self.set.append(lr_item)
	def union(self, item_set):
		for item in item_set:
			self.add(item)
	def __getitem__(self, index):
		return self.set[index]
	def __contains__(self, item):
		# Bit of a stretch for encapsulation...
		return item in self.set and self.set[self.set.index(item)].lookaheads.intersection(item.lookaheads)
	def __len__(self):
		return len(self.set)
	def __str__(self):
		return '\t'+'\n\t'.join([item.__str__() for item in self.set])
	def __repr__(self):
		return self.__str_
	def __eq__(self, item_set):
		return self.set == item_set.set
	def __hash__(self):
		return hash(tuple(self.set))
	def index(self, item):
		for i, _item in enumerate(self.set):
			if item == _item:
				return i

class SLR_collection:
	def __init__(self, *args):
		if args:
			self.collection = list(args[0].collection)
		else:
			self.collection = list()
	def add(self, item_set):
		if item_set not in self.collection:
			self.collection.append(item_set)
	def union(self, collection):
		for item_set in collection:
			self.add(item_set)
	def __getitem__(self, index):
		return self.collection[index]
	def __contains__(self, item):
		for i in self.collection:
			if i == item:
				return True
		return False
	def __len__(self):
		return len(self.collection)
	def __nonzero__(self):
		return len(self) > 0
	def index(self, value):
		return self.collection.index(value)
	def index_of_item(self, item):
		for i, item_set in enumerate(self.collection):
			if item in item_set:
				return i
	def __str__(self):
		return "Collection:\n"+ "\n\n".join(["I"+str(i)+str(set) for i, set in enumerate(self.collection)])
	def __repr__(self):
		return self.__str__()
	def get_LR_collection(self):
		collection = LR_collection()
		for set in self.collection:
			collection.add(set.get_LR_item_set())
		return collection

class LR_collection:
	def __init__(self, *args):
		if args:
			self.collection = list(args[0].collection)
		else:
			self.collection = list()
	def add(self, item_set):
		if item_set not in self.collection:
			self.collection.append(item_set)
	def union(self, collection):
		for item_set in collection:
			self.add(item_set)
	def __len__(self):
		return len(self.collection)
	def __iter__(self):
		return iter(self.collection)
	def __getitem__(self, index):
		return self.collection[index]
	def __str__(self):
		return "Collection:\n"+ "\n\n".join([str(i)+str(item_set) for i, item_set in enumerate(self.collection)])
	def __repr__(self):
		return self.__str__()
	def index_of_item(self, item):
		for i, item_set in enumerate(self.collection):
			for j, _item in enumerate(item_set):
				if item == _item:
					return i, j
	def __hash__(self):
		return hash(sum(hash(i) for i in self.collection))

class Parsing_table:
	def __init__(self, terminals, nonterminals):
		self.table = list()
		self.terminals = terminals
		self.nonterminals = nonterminals
	def append(self, row):
		self.table.append(row)
	def __len__(self):
		return len(self.table)
	def __getitem__(self, index):
		return self.table[index]
	def index(self, value):
		return self.collection.index(value)
	def __str__(self):
		n_terminals = len(self.terminals)
		n_nonterminals = len(self.nonterminals)
		l = "\u250C"+("\u2500"*7+"\u252C")+("\u2500")*(n_terminals*8-1)+\
			"\u252C"+"\u2500"*(n_nonterminals*8-1)+"\u2510\n"
		l += "\u2502 STATE \u2502 \t"
		l += "\t"*(n_terminals-n_terminals//2-2)+"      "\
			"ACTION"+"  "+"\t"*(n_terminals//2)
		l += "\u2502"+"\t "*(n_nonterminals-n_nonterminals//2-1)+\
			"      GOTO"+"\t"*(n_nonterminals//2)+"\u2502\n"
		l += "\u2502\t"
		l += "\u251C"+7*"\u2500"+("\u252C"+7*"\u2500")*(n_terminals-1)+"\u253C"+\
			(7*"\u2500"+"\u252C")*(n_nonterminals-1)+7*"\u2500"+"\u2524\n"
		l += "\u2502\t"
		l += "\t".join((f"\u2502 {terminal}" for terminal in self.terminals))
		l += "\t"+"\t".join((f"\u2502 {nonterminal}" for nonterminal in self.nonterminals))
		l += "\t\u2502\n"
		l += "\u251C"+("\u2500"*7+"\u253C")*(n_terminals+n_nonterminals)+"\u2500"*7+"\u2524\n"
		for i, r in enumerate(self.table):
			i = str(i)
			s = " "*(3-len(i))+i
			l += f"\u2502 {s}\t"
			for terminal in self.terminals:
				x = r.get(terminal)
				if x:
					if x > 0:
						l += f"\u2502 s{x}\t"
					else:
						l += f"\u2502 r{-(x+1)}\t"
				else:
					l += "\u2502\t"
			for nonterminal in self.nonterminals:
				x = r.get(nonterminal)
				if x:
					l += f"\u2502 {x}\t"
				else:
					l += "\u2502\t"
			l += "\u2502\n"
		l += "\u2514"+("\u2500"*7+"\u2534")*(n_terminals+n_nonterminals)+\
			"\u2500"*7+"\u2518"
		return l

class Production:
	def __init__(self):
		self.prod = set()
		self.nullable = False
	def get(self):
		return self.nullable
	def set(self, value):
		self.nullable = value
	def add(self, item):
		self.prod.add(item)
	def union(self, other_set):
		return self.prod.union(other_set)
	def update(self, other_set):
		self.prod.update(other_set)
	def __contains__(self, item):
		return item in self.prod
	def __len__(self):
		return len(self.prod)
	def __iter__(self):
		return iter(self.prod)
	def __str__(self):
		return str(self.prod)+" epsilon: "+str(self.nullable)
	def __repr__(self):
		return str(self)



#algorithms:
def construct_SLR_items_terminals_and_nonterminals():
	if len(sys.argv) < 2:
		print("To few arguments. Grammar file needed")
		quit()
	items = dict()
	nonterminals = set()
	terminals = {'$'}
	precedence = dict()
	prods = dict()
	nullable = dict()
	rules = list()
	prod_num = 0
	with open(sys.argv[1], "r", encoding="utf-8") as f:
		for i, line in enumerate(f):
			if line[:11] == "precedence:":
				preces = line[11:].split(',')
				for pre in preces:
					q = pre.split(':')
					precedence[q[0].strip()] = int(q[1].strip())
				continue
			prod = line.split('->')
			if prod[0] == '\n':
				continue
			prod_head = prod[0].strip()
			nonterminals.add(prod_head)
			if items.get(prod_head) == None:
				items[prod_head] = set()
				prods[prod_head] = set()
			or_prods = prod[1].strip().split(' | ')
			for or_prod in or_prods:
				symbols = or_prod.split(' ')
				rules.append((prod_head, symbols))
				if symbols[0] == '#':
					nullable[prod_head] = True
					prod_num += 1
				else:
					prods[prod_head].add(tuple(symbols))
					item = SLR_item(prod_head, symbols, 0, prod_num)
					items[prod_head].add(item)
					for i, symbol in enumerate(symbols):
						if (symbol[0] == "'" and symbol[-1] == "'"):
							terminals.add(symbol)
						else:
							nonterminals.add(symbol)
						item = SLR_item(prod_head, symbols, i+1, prod_num)
						items[prod_head].add(item)
					if nullable.get(prod_head) == None:
						nullable[prod_head] = False
					prod_num += 1
	return rules, items, prods, nonterminals, terminals, precedence, nullable

def generate_collection_and_GOTO_table(items, nonterminals, terminals, nullable):
	table = list()#Parsing_table(terminals, nonterminals)
	collection = SLR_collection()
	new_sets = SLR_collection()
	tmp_set = SLR_item_set()
	tmp_set.add(SLR_item("start", ['S'], 0, 0))
	new_sets.add(SLR_CLOSURE(tmp_set, items, nullable))
	while True:
		tmp_sets = SLR_collection()
		i = len(collection)+len(new_sets)
		for item_set in new_sets:
			row = dict()
			for terminal in terminals:
				tmp = SLR_GOTO(item_set, terminal, items, nullable)
				if tmp is not None:
					if tmp in collection:
						row[terminal] = collection.index(tmp)
					else:

						if tmp in new_sets:
							row[terminal] = new_sets.index(tmp)+len(collection)
						else:
							row[terminal] = i
							tmp_sets.add(tmp)
							i += 1

			for nonterminal in nonterminals:
				tmp = SLR_GOTO(item_set, nonterminal, items, nullable)
				if tmp is not None:
					if tmp in collection:
						row[nonterminal] = collection.index(tmp)
					else:
						if tmp in new_sets:
							row[nonterminal] = new_sets.index(tmp)+len(collection)
						else:
							row[nonterminal] = i
							tmp_sets.add(tmp)
							i += 1
			table.append(row)
		collection.union(new_sets)
		if not tmp_sets:
			break
		new_sets = SLR_collection(tmp_sets)
	return collection, table

def SLR_GOTO(slr_set, symbol, items, nullable):
	# The GOTO function for SLR explained in the dragon book
	next_set = SLR_item_set()
	for item in slr_set:
		next_symbol = item.next_symbol()
		if symbol == next_symbol:
			next_item = item.next_item()
			if next_item is not None:
				for prod_item in items[item.head]:
					if prod_item.body == next_item.body and \
					   prod_item.index == next_item.index:
					   next_set.add(prod_item)
					   break
	if next_set:
		return SLR_CLOSURE(next_set, items, nullable)
	return None

def SLR_CLOSURE(slr_set, items, nullable):
	# The CLOSURE function for SLR explained in the dragon book
	closure = SLR_item_set(slr_set)
	added = True
	while added:
		added = False
		tmp_set = SLR_item_set()
		for item in closure:
			next_symbol = item.next_symbol()
			if (items.get(next_symbol) != None):
					for _item in items[next_symbol]:
						if _item.index == 0 and _item not in closure:
							tmp_set.add(_item)
							while nullable.get(_item.next_symbol()):
								_item = _item.next_item()
								tmp_set.add(_item)
							else:
								tmp_set.add(_item)

							added = True
		closure.union(tmp_set)
	return closure

def SLR_kernel(slr_collection):
	kernel_collection = SLR_collection()
	kernel_set = SLR_item_set()
	for item in slr_collection[0]:
		if item.head == 'start':
			kernel_set.add(item)
		elif item.index > 0:
			kernel_set.add(item)
	kernel_collection.add(kernel_set)
	for item_set in slr_collection[1:]:
		kernel_set = SLR_item_set()
		for item in item_set:
			if item.index > 0:
				kernel_set.add(item)
		kernel_collection.add(kernel_set)
	return kernel_collection

def determine_lookahead_types(slr_kernel, goto_table, first, nullable):
	spontaneous = [[None]*len(item_set) for item_set in slr_kernel]
	propagator_table = [[None]*len(item_set) for item_set in slr_kernel]
	lr_kernel = slr_kernel.get_LR_collection()
	for i, kernel_set in enumerate(slr_kernel):
		GOTO = goto_table[i]
		for j, kernel in enumerate(kernel_set):
			if kernel.next_symbol() == None:
				continue
			LR_kernel = kernel.get_LR_item()
			LR_kernel.add('#')
			J = LR_CLOSURE(LR_kernel, first, nullable)
			propagators = set()
			for B in J:
				_i = GOTO[B.next_symbol()]
				_j = lr_kernel[_i].index(B.next_item())
				sponts = set()
				if '#' in B:
					propagators.add((_i, _j))
				for lookahead in B:
					if lookahead == '#':
						continue
					sponts.add(lookahead)
				if sponts:
					if spontaneous[_i][_j] == None:
						spontaneous[_i][_j] = sponts
					else:
						spontaneous[_i][_j].update(sponts)
			propagator_table[i][j] = propagators
	return lr_kernel, spontaneous, propagator_table

def LR_CLOSURE(lalr_set, first, nullable):
	closure = LR_item_set(lalr_set)
	added = True
	while added:
		added = False
		tmp_set = LR_item_set()
		for item in closure:
			next_symbol = item.next_symbol()
			if (items.get(next_symbol) != None):
				for slr_item in items[next_symbol]:
					if slr_item.index == 0:
						new_item = slr_item.get_LR_item()
						next_next_symbol = item.next_next_symbol()
						if next_next_symbol is not None:
							for terminal in first[next_next_symbol]:
								new_item.add(terminal)
							if nullable[next_next_symbol]:
								new_item.union(item.lookaheads)
						else:
							new_item.union(item.lookaheads)
						if new_item not in closure:
							tmp_set.add(new_item)
							added = True
		closure.union(tmp_set)
	return closure

def first_follow_nullable(productions, nonterminals, terminals, nullable):
	first = {nonterminal : set() for nonterminal in nonterminals}
	first.update({terminal : {terminal} for terminal in terminals})

	follow = {nonterminal : set() for nonterminal in nonterminals}
	follow.update({terminal : set() for terminal in terminals})
	nullable.update({terminal : False for terminal in terminals})
	follow['start'] = {'$'}
	last_hash = 0
	curr_hash = ffnhash(first, follow, nullable)

	while curr_hash != last_hash:
		for symbol, products in productions.items():
			for production in products:
				l = len(production)
				if l > 0:
					i = 0
					while i < l and nullable[production[i]]:
						first[symbol] |= first[production[i]]
						i += 1
					else:
						first[symbol] |= first[production[0]]
					if i == l:
						nullable[symbol] = True
					else:
						first[symbol] |= first[production[i]]
					i = l-1
					while i >= 0 and nullable[production[i]]:
						i -= 1
					follow[production[i]] |= follow[symbol]
					for i, b in enumerate(production):
						while i < l and nullable[production[i]]:
							i += 1
						if i < l-1:
							follow[b] |= first[production[i+1]]

		last_hash = curr_hash
		curr_hash = ffnhash(first, follow, nullable)

	return first, follow, nullable

def ffnhash(first, follow, nullable):
	return hash(sum(sum(hash(i) for i in s) for s in first.values())+\
			sum(sum(hash(i) for i in s) for s in follow.values())+\
			sum(i for i in nullable.values()))

def propagate_lookaheads(lr_kernel, spontaneous, propagator_table):
	lr_kernel[0][0].add('$')
	for i, l in enumerate(spontaneous):
		for j, spont in enumerate(l):
			if spont:
				lr_kernel[i][j].union(spont)
	last_hash = 0
	curr_hash = hash(lr_kernel)
	while last_hash != curr_hash:
		# Not exactly the same as in the dragon book, since the lr_kernel changes
		# during looping. There should be no difference in the result, just
		# fewer iterations
		for i, l in enumerate(propagator_table):
			for j, s in enumerate(l):
				if s:
					lookaheads = lr_kernel[i][j]
					for x, y in s:
						lr_kernel[x][y].union(lookaheads)
		last_hash = curr_hash
		curr_hash = hash(lr_kernel)

	return lr_kernel

def generate_parse_table(lalr_kernel, goto_table, terminals, nonterminals, first, nullable, precedence):
	table = Parsing_table(terminals, nonterminals)
	for i, row in enumerate(goto_table):
		tmp_row = dict()
		item_set = LR_CLOSURE(lalr_kernel[i], first, nullable)
		for terminal in terminals:
			x = row.get(terminal)
			if x:
				tmp_row[terminal] = x

			for item in item_set:
				if item.index == len(item.body):
					if terminal in item:
						x = tmp_row.get(terminal)
						if x:
							if x > 0:
								z = precedence.get(terminal)
								error = True
								if z != None:
									for s in item.body[::-1]:
										if s in terminals:
											q = precedence.get(s)
											if q != None:
													if z < q:
														tmp_row[terminal] = -1-item.prod_num
													error = False
													break
								if error:
									print(f"shift/reduce conflict on symbol " \
										f"{terminal} in:\n{item_set}.\n"
										f"Will resolve by shifting\n")

							else:
								print(f"reduce/reduce conflict on symbol {terminal} in\n{item_set}")
						else:
							tmp_row[terminal] = -1-item.prod_num
		for nonterminal in nonterminals:
			x = row.get(nonterminal)
			if x:
				tmp_row[nonterminal] = x
		table.append(tmp_row)
	return table

def LR_parsing_algorithm(parsing_table, reduction_rules, input):
	print("parsing...")

	stack = list()
	stack.append(0)
	input = "'" + input.replace(" ", "' '") + "'"
	input = input.split()
	if input[0] == "'V'":
		verbose = True
	else:
		verbose = False
	input = input[1:]
	i = 0
	while True:
		a = input[i] if i < len(input) else '$'
		action = parsing_table[stack[-1]].get(a)
		if verbose:
			s = str(stack)
			s1 = "".join(input[i:])
			print("STACK\t" + s[1:-1])
			print("INPUT\t" + s1+"$")
			print("ACTION", end="\t")
		if action == None:
			print(f"error in state {stack[-1]} on symbol {a}: stack {stack}")
			print("".join(input[i:]))
			break
		elif action >= 0:
			stack.append(action)
			if verbose:
				print(f"shift to {action}\n")
			i += 1
		else:
			if action == -1:
				print("accept\n\n")
				return
			else:
				r = reduction_rules[-(action+1)]
				for _ in range(len(r[1])):
					stack.pop()
				stack.append(parsing_table[stack[-1]].get(r[0]))
				if verbose:
					print("reduce by "+r[0] + " -> "+" ".join(r[1])+"\n")

rules, items, prods, nonterminals, terminals, pres, nullable = construct_SLR_items_terminals_and_nonterminals()
collection, table = generate_collection_and_GOTO_table(items, nonterminals, terminals, nullable)


slr_kernel = SLR_kernel(collection)
first, follow, nullable = first_follow_nullable(prods, nonterminals, terminals, nullable)

lr_kernel, spontaneous, propagator_table = determine_lookahead_types(slr_kernel, table, first, nullable)

lr_kernel = propagate_lookaheads(lr_kernel, spontaneous, propagator_table)

parse_table = generate_parse_table(lr_kernel, table, terminals, nonterminals, first, nullable, pres)
for i, rule in enumerate(rules):
		print(f"({i}) {rule[0]}" + " -> "+" ".join(rule[1]))
print(collection)
print(parse_table)
print(sys.getsizeof(parse_table.table))
while True:
	i = input().strip()
	LR_parsing_algorithm(parse_table, rules, i)
