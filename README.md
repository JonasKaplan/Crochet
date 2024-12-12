# Crochet

Crochet is an interpreted, esoteric programming language. It is designed to have an extremely minimal, highly structured syntax, as well as a consistent modal of computation.

## Language

Below is an example of a Crochet program, found in the `/Examples` directory, which calculates the 20th fibonacci number

```cht
# Crochet demo

origin
    spawn
        _ -> 20 fib
    pop
        _ -> @ ! 0

fib
    spawn
        0 -> 0
        1 -> 1
        _ -> @ -1 fib -1 fib 0
    pop
        _ -> +@
```

### Syntax

The individual code blocks are referred to as "nodes". The first line of a node is its name, which can be referenced elsewhere. The only reserved keywords in Crochet are `spawn` and `pop`, so any other word is valid. Names must start with an alphabetic character, but can contain any characters otherwise.

Every node must have a `spawn` block and a `pop` block. The lines in the `spawn` and `pop` blocks are referred to as "rules". Each rule dictates the node's behaviour under some condition.

### Behaviour

A node always has a single internal value that it keeps track of, stored as a 64 bit unsigned integer. This value is always 0 initially. When a node is created, it looks at the rules in the `spawn` block, and applies the rule based on the current value of the node that instantiated it. That is to say, if the node that created it had an internal value of 3, the created node would apply the spawn rule corresponding to the number 3.

The first component of a rule is the match, which is the number required for it to be invoked. This can be a number, or a wildcard `_`, which matches if none of the other rules do. There must be exactly one wildcard rule for every `spawn` and `pop` block.

The actual content of the rule is a series of actions. A number on its own sets the internal value of the node to that number. For example, in the line `_ -> 20 fib`, The first action sets the node's value to 20. An operation preceding a number applies that operation between the node's internal value and the constant. For example, on the line `_ -> @ -1 fib -1 fib`, each `-1` subtracts 1 from the node's value.

There are also two special numbers that can be operated on. `@` represents the value of the rule that was applied. This is especially useful with `_` wildcard rules. `&` represents user input, and reads a single byte from `stdin`.

There are two further actions that can be taken. The first is `!`, which prints the internal value of the node to `stdout`. The second is to create a child node, simply by invoking its name. The child node will be created with the internal value of the calling node when it is invoked.

Execution always starts at the node called `origin`. The order in which nodes are declared is irrelevant, they can always be executed from anywhere.

Pop rules are invoked when a child node finishes execution. When this happens, the rule corresponding to the internal value of the child when it finishes execution is executed. For example, In the rule `_ -> +@`, the internal value of the child is simply added to the parent, regardless of what it is.

Now, to go through an example rule: `_ -> @ -1 fib -1 fib 0`. This rule says that for any number, set the node's internal value to that number, then subtract 1 and invoke fib, then do so again, then set the node's value to 0.

Every node executes on its own thread. This means the performance of the language is terrible. It also means that the order in which pop rules are invoked is not guaranteed, which means memoization of node invocations is impossible. A node simply keeps track of the number of currently active children it has spawned, and returns its value to its parent when that number reaches 0. Below is a silly example

```cht
origin
    spawn
        _ -> random
    pop
        _ -> @ ! 0

random
    spawn
        _ -> one two
    pop
        _ -> @

one
    spawn
        _ ->  1
    pop
        _ -> @

two
    spawn
        _ -> 2
    pop
        _ -> @
```

This program's behaviour is impossible to determine, because the order of execution is not guaranteed (although in practice, this is not the case, since starting threads takes time).

## Credits

All code and documentation created by Jonas M. Kaplan.
