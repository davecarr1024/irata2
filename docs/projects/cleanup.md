# general principles

* Generally be more defensive and put things in protected or private namespaces in classes unless they have to be public. Don't expose things just for testing but test throuogh the public interface. Design for testability.
* One class per file.
* Nest directories in modules - for example alu under sim. Group classes by functionality where reasonable. Use nested namespaces to match - ex irata2::sim::alu::Alu. Where there's a complex cpu component like alu, memory, status, controller, etc, that should be a submodule.
* Top level modules should have a docs/ directory with a design doc for each submodule and a top-level readme.md.

# sim

## Component

* current_phase(), RegisterChild(), Tick*() should be protected or private

## ComponentWithParent

* RegisterChild() should be called by each ComponentWithParent during its construction. 
* Each Component should keep a list of its children and use that to propagate ticks downward, including Cpu.
* All structural references: parent_, children_, etc should be constant and set during the constructor. 
* Use factory pattern to handle parents that contain children that have references back to parent. A factory that takes in parent and builds child in place. Use this for alu modules, memory regions, memory modules inside of regions, etc.

## Cpu

* RunResult should contain full optional debug dump of the cpu state and an enum for how halting happened: timeout, crash, halt.
* Cpu constructor should by default do a static lookup of the hdl and compiled microcode. Cpu shouldn't be possible to build with any other hdl or microcode. This current ctor is a violation of "only build an irata cpu".
* The static methods that build a singleton hdl and microcode program should live as private static methods in cpu. This generally replaces the initialization source file.
* The cpu should validate itself that it matches the hdl (or the hdl can implement the validation either way).
* The cpu constructor should accept the cartridge as a Rom and an optional debug symbols object, and extra regions for mmio.
* Caching control and status lookups in cpu is fine for performance.

## ComponentWithBus

* This class should hold the bus reference, read control, and write control like it currently does. 
* There should be abstract protected read()->ValueType and write(ValueType) methods that are called by the TickRead and TickWrite implementations in ComponentWithBus.
* This allows for generalizing interaction with the bus for _all_ bus-connected components including registers and more complicatd things like memory.

## Controls

* There should be a root Control type that is very general and virtual.
* All client uses of Controls should be through a leaf type. There should be Latched<phase>Control for latched controls and <phase>Control for auto-reset controls. It's ok to have a AutoResetControl and LatchedControl intermediate type in the hierarchy for clarity. I like keeping the information in the templates like it is now.

## Registers

* registers should have a ValueType template
* registers should by default not be connected to a bus
* RegisterWithBus should be the version connected to a bus and should implement ComponentWithBus
* registers should provide a reset control by default
* WordRegister has its own type hierarchy. There are several types of WordRegisters but they all have a high and low ByteRegister and a word-level reset control. I think by default WordRegisters should require a word bus that the word as a whole is connected to, and a byte register that the bytes are connected to. 
* There should be a cpu-level tmp word register that's connected to both buses
* Double-connecting word registers is useful. Like consider the MAR, it's useful to move the word as a whole from the PC to the MAR, or to load bytes one at a time from memory into the MAR (or tmp) to do absolute or other fancier addressing modes.
* Make a WordCounter type that is a word register with ByteRegister bytes but also has a word level increment control that increments the low byte and handles overflow from both bytes.
* LatchedWordRegister is a good idea but I don't like the implementation. It should be a word register that isn't connected to a bus but that has a hard connection to another word register, and it should have a latch control that latches the value from the target register into the LatchedWordRegister. This would make it easy and contained to implement IPC.

## Controller

* Controller should be a sim submodule and it will have a whole tree of components to implement the controller.
* The curent controller implementation isn't hardware-ish. It stores the microcode program directly and accesses it directly. This isn't the intended implementation.
* The ControlEncoder and StatusEncoder and a combined InstructionEncoder should all be components that are implemented in the controller submodule and are attached to the InstructionMemory.
* The InstructionMemory is a smart memory object that contains the encoders and a set of Rom objects. Since Rom objects have a 16 bit addressing space and a 8 bit data width, there will need to be a dynamic grid of Roms generated to cover the needed instruction address and data spaces. This should be implemented in InstructionMemory. The top level interface of InstructionMemory is a lookup based on opcode, step, and statuses that yields a set of sim control references.
* InstructionMemory takes in the microcode program as a constructor argument (forwarded down from controller and generated by the cpu constructor statically) but _does not store it_. It stores the information encoded in roms in a hardware-ish way.
* The ControlEncoder does a few things. It takes in the microcode program but doesn't store it. It gets the full set of controls used in the microcode and fetches the corresponding set of controls from the sim and stores the references to them. It implements encode/decode logic to go from an encoded control word to a set of sim control references and vice versa.
* The StatusEncoder is similar and does a lookup of all the statuses from the microcode program in the sim component tree at startup. It implements encoding from a CompleteStatus object to a binary encoding based on a stable status ordering, and vice versa. The StatusEncoder also supports taking in a PartialStatus object that is a partially specified status value vector and permuting it into all the matching CompleteStatus objects based on the full status space. These are all implemented in the controller submodule.
* At runtime, the controller itself pulls state from its ir and sc registers, gathers status state, fetches the control set from the instructionMemory based on those values, and asserts those controls. It contains very little internal control and serves as a connection for the internal component to the wider system.

## alu

* The alu is in pretty good shape. I'll review this later in more detail but for now it looks good.

## memory

* A memory region should be a ComponentWithParent. This introduces an initialization interdependency but that should be resolved by changing the constructor regions arg in memory to be a vector of region constructors, each of which takes the memory parent object as an argument. 
* The same logic applies to modules. They should be a ComponentWithParent and the Region constructor should take one module factory that builds the module with the reference to the region.
* Memory should be a ComponentWithBus.

# hdl

* Generally good, but the hdl should enforce at startup that the sim is a superset of the hdl.
* All the structural changes above should be reflected and enforced in the hdl.