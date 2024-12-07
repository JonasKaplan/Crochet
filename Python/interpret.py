# This is mostly here for historical reasons
# Written in a fervor between 4am and 6am as a proof of concept
# It doesn't stick to the recent version of the language
# To use it, replace `@` with `it`, and `!` with `show`
# Also, there's no comments, so `#` will break it as well

from pathlib import Path
from typing import Callable
from enum import Enum
from threading import Thread

class ParseException(Exception):
    pass

class RuntimeException(Exception):
    pass

class RuleAction(Enum):
    Set = 0
    Call = 1
    Add = 2
    Subtract = 3
    Multiply = 4
    Divide = 5
    It = 6
    Show = 7

class Rule:
    def __init__(self, source: str, spawn: bool) -> None:
        tokens: list[str] = source.split(" ")
        if (tokens[1] != "->"):
            raise ParseException(f"Expected spawn rule arrow, found \"{tokens[1]}\"")
        self.action_list: list[tuple[RuleAction, int] | tuple[RuleAction, str] | tuple[RuleAction, RuleAction]] = []
        action: RuleAction
        for token in tokens[2:]:
            if (token[0].isdigit()):
                if (not token.isdecimal()):
                    raise ParseException(f"Expected numeric literal, found \"{token}\"")
                self.action_list.append((RuleAction.Set, int(token)))
            elif (token[0].isalpha()):
                if (token == "it"):
                    if (spawn):
                        raise ParseException("Cannot invoke `it` in spawn rule")
                    self.action_list.append((RuleAction.Set, RuleAction.It))
                elif (token == "show"):
                    self.action_list.append((RuleAction.Show, RuleAction.Show)) #ok
                else:
                    self.action_list.append((RuleAction.Call, token))
            else:
                match token[0]:
                    case '+':
                        action = RuleAction.Add
                    case '-':
                        action = RuleAction.Subtract
                    case '*':
                        action = RuleAction.Multiply
                    case '/':
                        action = RuleAction.Divide
                    case _:
                        raise ParseException(f"Expected operator, found \"{token}\"")
                if (not token[1:].isdecimal()):
                    if (token[1:] == "it"):
                        if (spawn):
                            raise ParseException("Cannot invoke `it` in spawn rule")
                        self.action_list.append((action, RuleAction.It))
                    else:
                        raise ParseException(f"Expected numeric literal, found \"{token}\"")
                else:
                    self.action_list.append((action, int(token[1:])))


class Node:
    def __init__(self, source: list[str]) -> None:
        self.name: str = source[0]
        if (source[1] != "spawn"):
            raise ParseException(f"Expected spawn rule marker, found \"{source[1]}\"")
        self.spawn_rules: dict[int, Rule] = {}
        temp_default_spawn_rule: Rule | None = None
        int_str: str
        pop_index: int
        try:
            pop_index = source.index("pop")
        except ValueError:
            raise ParseException("No pop rule marker found")
        for rule in source[2:pop_index]:
            if (rule[0].isdigit()):
                int_str = rule.split(" ")[0]
                if (not int_str.isdecimal()):
                    raise ParseException(f"Expected numeric literal, found \"{int_str}\"")
                self.spawn_rules[int(int_str)] = Rule(rule, True)
            elif (rule[0] == "_"):
                if (temp_default_spawn_rule is not None):
                    raise ParseException(f"Too many default rules")
                temp_default_spawn_rule = Rule(rule, True)
            else:
                raise ParseException(f"Expected spawn rule, found \"{rule}\"")
        if (temp_default_spawn_rule is None):
            raise ParseException(f"Expected default spawn rule")
        self.default_spawn_rule: Rule = temp_default_spawn_rule
        self.pop_rules: dict[int, Rule] = {}
        temp_default_pop_rule: Rule | None = None
        for rule in source[(pop_index + 1):]:
            if (not rule[0].isdigit() and (rule[0] != "_")):
                break
            if (rule[0].isdigit()):
                int_str = rule.split(" ")[0]
                if (not int_str.isdecimal()):
                    raise ParseException(f"Expected numeric literal, found \"{int_str}\"")
                self.pop_rules[int(int_str)] = Rule(rule, False)
            elif (rule[0] == "_"):
                if (temp_default_pop_rule is not None):
                    raise ParseException(f"Too many default rules")
                temp_default_pop_rule = Rule(rule, False)
            else:
                raise ParseException(f"Expected pop rule, found \"{rule}\"")
        if (temp_default_pop_rule is None):
            raise ParseException(f"Expected default pop rule")
        self.default_pop_rule: Rule = temp_default_pop_rule

    def spawn(self, value: int, nodes: dict[str, "Node"], ret: list[int]) -> None:
        spawn_rule: Rule = self.spawn_rules.get(value, self.default_spawn_rule)
        threads: list[tuple[Thread, list[int]]] = []
        i: int = 0
        while (i < len(spawn_rule.action_list)):
            current, next = spawn_rule.action_list[i]
            match current:
                case RuleAction.Set:
                    value = next
                case RuleAction.Call:
                    if (next not in nodes):
                        raise RuntimeException(f"No such node \"{next}\"")
                    thread_ret: list[int] = []
                    threads.append((Thread(target = nodes[next].spawn, args = (value, nodes, thread_ret)), thread_ret))
                    threads[-1][0].start()
                case RuleAction.Add:
                    value += next
                case RuleAction.Subtract:
                    value -= next
                case RuleAction.Multiply:
                    value *= next
                case RuleAction.Divide:
                    value //= next
                case RuleAction.Show:
                    print(value)
            i = i + 1
        pop_rule: Rule
        thread: tuple[Thread, list[int]]
        while (len(threads) != 0):
            thread = threads.pop(0)
            thread[0].join()
            pop_rule = self.pop_rules.get(thread[1][0], self.default_pop_rule)
            i = 0
            while (i < len(pop_rule.action_list)):
                current, next = pop_rule.action_list[i]
                match current:
                    case RuleAction.Set:
                        value = thread[1][0] if (next == RuleAction.It) else next
                    case RuleAction.Call:
                        if (next not in nodes):
                            raise RuntimeException(f"No such node \"{next}\"")
                        thread_ret: list[int] = []
                        threads.append((Thread(target = nodes[next].spawn, args = (value, nodes, thread_ret)), thread_ret))
                        threads[-1][0].start()
                    case RuleAction.Add:
                        value += thread[1][0] if (next == RuleAction.It) else next
                    case RuleAction.Subtract:
                        value -= thread[1][0] if (next == RuleAction.It) else next
                    case RuleAction.Multiply:
                        value *= thread[1][0] if (next == RuleAction.It) else next
                    case RuleAction.Divide:
                        value //= thread[1][0] if (next == RuleAction.It) else next
                    case RuleAction.Show:
                        print(value)
                i = i + 1
        ret.append(value)

def interpret(file_path: Path) -> None:
    lines: list[str] = []
    with open(file_path, "r") as f:
        lines = f.readlines()
    nodes: dict[str, Node] = {}
    lines = list(
        map(lambda line: line.strip(),
        filter(lambda line: not (line.isspace() or (line == "")),
        lines))
    )
    for i, line in enumerate(lines):
        if ((line != "spawn") and (line != "pop") and (not line[0].isdigit()) and (line[0] != "_")):
            try:
                nodes[line] = Node(lines[i:])
            except ParseException as e:
                print(f"Failed to parse node {line}\nmessage: {e}")
                return
    if ("origin" not in nodes):
        print("No entry point found")
        return
    print("Parsing successful, beginning interpretation")
    ret: list[int] = []
    nodes["origin"].spawn(20, nodes, ret)
    print(f"Program finished with exit code {ret[0]}")

interpret(Path("Example.cht"))