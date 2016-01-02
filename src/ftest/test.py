import language_vector

builder = language_vector.make_builder(3, 10000, 42)
a = language_vector.build(builder, "I like birds")
b = language_vector.build(builder, "I like jam")
language_vector.merge(a, language_vector.build(builder, "and some more things"))

print("a.a = %f" % language_vector.score(a, a))
print("b.b = %f" % language_vector.score(b, b))
print("a.b = %f" % language_vector.score(a, b))
print("b.a = %f" % language_vector.score(b, a))
