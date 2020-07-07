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

class hierarchy:
	item
   /     \
SLR-item  \
		LR_item
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
	def next_symbol(self):
		index = self.index
		if index < self.length:
			return self.body[index]
		return None

class SLR_item(item):
	def __init__(self, head, body, index, prod_num):
		super().__init__(head, body, index, prod_num)
	def next_item(self):
		index = self.index
		if index < self.length:
			return (self.body, index+1)
		return None
	def __eq__(self, item):
		return self.head == item.head and self.body == item.body and self.index == item.index
	def get_LR_item(self):
		return LR_item(self.head, self.body, self.index, self.prod_num)
	def __str__(self):
		s1 = " ".join(self.body[:self.index])
		s2 = " ".join(self.body[self.index:])
		return (f"[{self.head} -> {s1}. "
		       f"{s2}]")
	def __repr__(self):
		return self.__str__()
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
	def __str__(self):
		s1 = " ".join(self.body[:self.index])
		s2 = " ".join(self.body[self.index:])
		s3 = "/".join(self.lookaheads)
		return (f"[{self.head} -> {s1}. "
		       f"{s2}, {s3}]")
	def __repr__(self):
		return self.__str__()
	def __eq__(self, item):
		return self.head == item.head and self.body == item.body and \
			   self.index == item.index
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

class LR_item_set():
	def __init__(self, *args):
		if args:
			arg = args[0]
			if type(arg) == LR_item_set:
				self.set = list(arg.set)
			elif type(arg) == LR_item:
				self.set = [arg]
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
		if not item in self.set:
			return False
		elif not self.set[self.set.index(item)].lookaheads.intersection(item.lookaheads):
			return False
		return True
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

class SLR_item_set:
	def __init__(self, *args):
		if args:
			self.set = list(args[0].set)
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
		return '\t'+'\n\t'.join([item.__str__() for item in self.set])
	def __repr__(self):
		return self.__str__()
	def __hash__(self):
		return hash(tuple(self.set))

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
		return "Collection:\n"+ "\n\n".join([str(i) for i in self.collection])
	def __repr__(self):
		return self.__str__()

class Parsing_table:
	def __init__(self):
		self.table = list()
	def append(self, row):
		self.table.append(row)
	def __len__(self):
		return len(self.table)
	def __getitem__(self, index):
		return self.table[index]
	def index(self, value):
		return self.collection.index(value)
	def __str__(self):
		s = '\n'.join((str(i) for i in self.table))
		return s

class custom_set:
	def __init__(self, *args):
		if args:
			self.set = list(args[0].collection)
		else:
			self.set = list()
	def add(self, item_set):
		if item_set not in self.set:
			self.set.append(item_set)
	def union(self, collection):
		for item_set in collection:
			self.add(item_set)
	def __getitem__(self, index):
		return self.set[index]
	def __len__(self):
		return len(self.set)
	def __str__(self):
		s = ", ".join([i.__str__() for i in self.set])
		return f"{{ {self.set} }}"
	def __repr__(self):
		return self.__str__()
#algorithms:
def construct_SLR_items_terminals_and_nonterminals():
	if len(sys.argv) < 2:
		print("To few arguments. Grammar file needed")
		quit()
	items = dict()
	nonterminals = set()
	terminals = {'$'}
	prods = dict()
	rules = list()
	prod_num = 0
	with open(sys.argv[1], "r", encoding="utf-8") as f:
		for i, line in enumerate(f):
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
				rules.append((prod_head, or_prod))
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
				prod_num += 1
	return rules, items, prods, nonterminals, terminals

def generate_collection_and_GOTO_table(items, nonterminals, terminals):
	table = Parsing_table()
	collection = SLR_collection()
	new_sets = SLR_collection()
	tmp_set = SLR_item_set()
	tmp_set.add(SLR_item("start", ['S'], 0, 0))
	new_sets.add(SLR_CLOSURE(tmp_set, items))
	while True:#for _ in range(2):
		tmp_sets = SLR_collection()
		i = len(collection)+len(new_sets)
		for item_set in new_sets:
			row = dict()
			for terminal in terminals:
				tmp = SLR_GOTO(item_set, terminal, items)

				if tmp is not None:
					if tmp in collection:
						row[terminal] = (0, collection.index(tmp))
					else:

						if tmp in new_sets:
							print("hej")
							row[terminal] = (0, new_sets.index(tmp)+len(collection))
						else:
							row[terminal] = (0, i)
							tmp_sets.add(tmp)
							i += 1

			for nonterminal in nonterminals:
				tmp = SLR_GOTO(item_set, nonterminal, items)
				if tmp is not None:
					if tmp in collection:
						row[terminal] = (0, collection.index(tmp))
					else:
						if tmp in new_sets:
							row[nonterminal] = (0, new_sets.index(tmp)+len(collection))
						else:
							row[nonterminal] = (0, i)
							tmp_sets.add(tmp)
							i += 1
			table.append(row)
		collection.union(new_sets)
		if not tmp_sets:
			break
		new_sets = SLR_collection(tmp_sets)
	return collection, table

def SLR_GOTO(slr_set, symbol, items):
	# The GOTO function for SLR explained in the dragon book
	next_set = SLR_item_set()
	for item in slr_set:
		next_symbol = item.next_symbol()
		if symbol == next_symbol:
			next_item = item.next_item()
			if next_item != None:
				for prod_item in items[item.head]:
					if prod_item.body == next_item[0] and \
					   prod_item.index == next_item[1]:
					   next_set.add(prod_item)
					   break
	if next_set:
		return SLR_CLOSURE(next_set, items)
	return None

def SLR_CLOSURE(slr_set, items):
	# The CLOSURE function for SLR explained in the dragon book
	closure = SLR_item_set(slr_set)
	added = True
	while added:
		added = False
		tmp_set = SLR_item_set()
		for item in closure:
			next_symbol = item.next_symbol()
			if (items.get(next_symbol) != None):
				for item in items[next_symbol]:
					if item.index == 0 and item not in closure:
						tmp_set.add(item)
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

class Lookahead_table:
	def __init__(self, size):
		self.propagate_table = list()
		self.spontaneous_table = [None]*size

	def append(self, items):
		self.propagate_table.append(items)

	def insert_spont(self, index, items):
		self.spontaneous_table[index].append(items)

def determine_lookahead_types(slr_kernel, goto_table, first, nullable):
	table = Lookahead_table(len(slr_kernel))
	for i, kernel_set in enumerate(slr_kernel):
		for kernel in kernel_set:
			LR_kernel = kernel.get_LR_item()
			LR_kernel.add('#')
			J = LR_CLOSURE(LR_kernel, first, nullable)
			print()
			print(f"Closure on {LR_kernel}: {{ \n{J}}}")
			#for B in J:
			#	print(B)

def LR_CLOSURE(lalr_set, first, nullable):
	closure = LR_item_set(lalr_set)
	added = True
	while added:
		print()
		added = False
		tmp_set = LR_item_set()
		for item in closure:
			#print("Closure item", item)
			next_symbol = item.next_symbol()
			#print(next_symbol)
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

def first_follow_nullable(productions, nonterminals, terminals):
	first = {nonterminal : set() for nonterminal in nonterminals}
	first.update({terminal : {terminal} for terminal in terminals})

	follow = {nonterminal : set() for nonterminal in nonterminals}
	follow.update({terminal : set() for terminal in terminals})
	nullable = {nonterminal : False for nonterminal in nonterminals}
	nullable.update({terminal : False for terminal in terminals})
	follow['start'] = {'$'}
	last_hash = 0
	curr_hash = ffnhash(first, follow, nullable)
	while curr_hash != last_hash:
		for symbol, products in productions.items():
			for production in products:
				l = len(production)
				i = 0
				while i < l and nullable[production[i]]:
					i += 1
				if i == l:
					nullable[symbol] = True
					continue

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

def propagate_lookaheads(unfinished_lalr_kernel, lookahead_table):
	pass
def generate_parse_table(lalr_kernels, goto_table):
	pass
def LR_GOTO():
	pass


rules, items, prods, nonterminals, terminals = construct_SLR_items_terminals_and_nonterminals()
collection, table = generate_collection_and_GOTO_table(items, nonterminals, terminals)
"""for k, v in items.items():
	s = "\n\t".join([i.__str__() for i in v])
	print(f"{k}:\t{s}")"""
print(collection)
slr_kernel = SLR_kernel(collection)
first, follow, nullable = first_follow_nullable(prods, nonterminals, terminals)
print(first)
print(follow)
print(nullable)
determine_lookahead_types(slr_kernel, table, first, nullable)
