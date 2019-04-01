import gdb

def deref(reference):
	target = reference.dereference()
	if str(target.address) == '0x0':
		return 'NULL'
	else:
		return target


class TriePrinter(object):
	def __init__(self, val):
		self.val = val
	
	def to_string(self):
		return "{ root addr = 0x%x }" % self.val['root']
	
	def children(self):
		val = self.val
		yield ('root', val['root'])


class TrieNodePrinter(object):
	def __init__(self, val):
		self.val = val
	
	def to_string(self):
		return "{ addr = 0x%x, leaf = %s, children = %s }" \
			% (self.val, "True" if (self.val['isLeaf'] == 1) else "False", self.children_to_string())
	
	def children(self):
		val = self.val
		for i in range(26):
			if val['children'][i]:
				yield (("%c (0x%x)" % ((ord('A') + i), val['children'][i].address)), val['children'][i])
	
	def children_to_string(self):
		val = self.val
		children_str = ""
		for i in range(26):
			if val['children'][i]:
				if children_str != "":
					children_str += ", "
				children_str += "%c (0x%x)" % ((ord('A') + i), val['children'][i].address)
		return "{ %s }" % children_str


class LinkedTrieNodePrinter(object):
	def __init__(self, val):
		self.val = val
	
	def to_string(self):
		return "{ node = 0x%x }" % (self.val['node'])
	
	def children(self):
		val = self.val
		if val['next']:
			yield ('next', val['next'])


def type_filter(val):
	typename = str(val.type.strip_typedefs().unqualified())
	
	if (typename == 'Trie' or
		typename == 'Trie &'):
		return TriePrinter(val)
	if typename == 'TrieNode *':
		return TrieNodePrinter(val)
	if typename == 'LinkedTrieNode *':
		return LinkedTrieNodePrinter(val)
	
	return None


gdb.pretty_printers.append(type_filter)
