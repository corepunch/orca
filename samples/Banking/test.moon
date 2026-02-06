export my_func
x = 2323

collection =
  height: 32434
  hats: {"tophat", "bball", "bowler"}

my_func = (a) -> x + a

print my_func 100

test1 = (args, ...) ->
  for k, v in pairs args 
    print(k, v)
  for _, func in ipairs {...}
    func()

class Thing
  name: "unknown"

class Person extends Thing
  say_name: => 
    print "Hello, I am #{@name}!"

with Person!
  .name = "MoonScript"
  \say_name!
