# Flecs Types - Documentation Index

> **Complete documentation for Godot-Flecs ECS integration**

## 📑 Documentation Files

### Core Documentation

| Document | Description | Audience |
|----------|-------------|----------|
| **[README.md](./README.md)** | Overview and architecture | All users |
| **[QUICK_REFERENCE.md](./QUICK_REFERENCE.md)** | One-page cheat sheet | All users |
| **[FLECS_SERVER_API.md](./FLECS_SERVER_API.md)** | Complete API reference | Developers |

### Component Guides

| Document | Description | Audience |
|----------|-------------|----------|
| **[QUERY_API.md](./QUERY_API.md)** | Query system usage | Developers |
| **[QUERY_IMPLEMENTATION_README.md](./QUERY_IMPLEMENTATION_README.md)** | Query internals | Advanced |
| **[QUERY_VS_SCRIPTSYSTEM.md](./QUERY_VS_SCRIPTSYSTEM.md)** | When to use what | Developers |

### Performance & Optimization

| Document | Description | Audience |
|----------|-------------|----------|
| **[PERFORMANCE_FIX.md](./PERFORMANCE_FIX.md)** | Optimization guide | All users |
| **[BUILD_SYSTEM_REFACTORING.md](./BUILD_SYSTEM_REFACTORING.md)** | System refactoring details | Advanced |
| **[CURSOR_CONVERSION_README.md](./CURSOR_CONVERSION_README.md)** | Serialization internals | Advanced |

---

## 🎯 Quick Navigation

### I want to...

**Get started quickly**
→ Read [README.md](./README.md) Quick Start section

**Look up an API method**
→ Check [FLECS_SERVER_API.md](./FLECS_SERVER_API.md) or [QUICK_REFERENCE.md](./QUICK_REFERENCE.md)

**Optimize performance**
→ Read [PERFORMANCE_FIX.md](./PERFORMANCE_FIX.md)

**Understand queries**
→ Read [QUERY_API.md](./QUERY_API.md)

**Choose between query/system**
→ Read [QUERY_VS_SCRIPTSYSTEM.md](./QUERY_VS_SCRIPTSYSTEM.md)

**Understand code structure**
→ Read [BUILD_SYSTEM_REFACTORING.md](./BUILD_SYSTEM_REFACTORING.md)

---

## 📦 Class Documentation

All classes have comprehensive inline documentation using Doxygen-style comments:

### FlecsServer
- **Header**: `flecs_server.h` (with inline comments)
- **Documentation**: [FLECS_SERVER_API.md](./FLECS_SERVER_API.md)
- **Purpose**: Central singleton managing all ECS resources

### FlecsScriptSystem
- **Header**: `flecs_script_system.h` (fully documented)
- **Documentation**: Inline Doxygen comments in header
- **Purpose**: GDScript-accessible ECS system with advanced features

### FlecsQuery
- **Header**: `flecs_query.h` (documented in existing files)
- **Documentation**: [QUERY_API.md](./QUERY_API.md), [QUERY_IMPLEMENTATION_README.md](./QUERY_IMPLEMENTATION_README.md)
- **Purpose**: High-performance entity queries with caching

### Flecs Variant Types
- **Header**: `flecs_variant.h` (fully documented with Doxygen)
- **Documentation**: Inline Doxygen comments in header
- **Purpose**: RID-compatible wrappers for Flecs types

---

## 📚 Documentation by Experience Level

### Beginner
1. [README.md](./README.md) - Start here
2. [QUICK_REFERENCE.md](./QUICK_REFERENCE.md) - Common operations
3. [QUERY_API.md](./QUERY_API.md) - Basic queries
4. [PERFORMANCE_FIX.md](./PERFORMANCE_FIX.md) - Simple optimizations

### Intermediate
1. [FLECS_SERVER_API.md](./FLECS_SERVER_API.md) - Complete API
2. [QUERY_VS_SCRIPTSYSTEM.md](./QUERY_VS_SCRIPTSYSTEM.md) - Design patterns
3. Header files - Inline documentation
4. `example_query_usage.gd` - Working examples

### Advanced
1. [BUILD_SYSTEM_REFACTORING.md](./BUILD_SYSTEM_REFACTORING.md) - Code architecture
2. [QUERY_IMPLEMENTATION_README.md](./QUERY_IMPLEMENTATION_README.md) - Internals
3. [CURSOR_CONVERSION_README.md](./CURSOR_CONVERSION_README.md) - Serialization
4. Source code - Implementation details

---

## 🎓 Learning Path

```
1. Read README.md (Overview & Quick Start)
   ↓
2. Follow Quick Start examples
   ↓
3. Use QUICK_REFERENCE.md as cheat sheet
   ↓
4. Read PERFORMANCE_FIX.md for optimization
   ↓
5. Study FLECS_SERVER_API.md for advanced features
   ↓
6. Explore specialized docs as needed
```

---

## 🔍 Finding Specific Information

### API Methods
**Location**: [FLECS_SERVER_API.md](./FLECS_SERVER_API.md)
**Search**: Ctrl+F for method name

### GDScript Examples
**Location**: [README.md](./README.md) or `example_query_usage.gd`
**Search**: Look for ```gdscript blocks

### Performance Tips
**Location**: [PERFORMANCE_FIX.md](./PERFORMANCE_FIX.md)
**Search**: Look for ✅ DO / ❌ DON'T sections

### Class Structure
**Location**: Header files or [BUILD_SYSTEM_REFACTORING.md](./BUILD_SYSTEM_REFACTORING.md)
**Search**: Member variable or method name

---

## 📖 Documentation Standards

All documentation follows these standards:

✅ **Markdown format** for readability  
✅ **Code examples** with syntax highlighting  
✅ **Clear section headers** for navigation  
✅ **Tables** for comparisons  
✅ **Emoji** for visual landmarks  
✅ **Doxygen comments** in header files  

---

## 🔄 Recently Updated

- **2025**: Complete documentation overhaul
- All headers now have Doxygen comments
- New FLECS_SERVER_API.md comprehensive reference
- New QUICK_REFERENCE.md cheat sheet
- BUILD_SYSTEM_REFACTORING.md documenting code cleanup

---

## 💡 Tips for Using Documentation

1. **Start with README.md** - Don't skip the overview
2. **Keep QUICK_REFERENCE.md handy** - It's your cheat sheet
3. **Search before asking** - Most answers are documented
4. **Read headers** - Inline docs are often more detailed
5. **Try examples** - `example_query_usage.gd` has working code
6. **Check performance docs** - They have benchmarks and tips

---

## 🆘 Still Can't Find What You Need?

1. Check the [README.md](./README.md) troubleshooting section
2. Search all docs with `grep -r "search term" *.md`
3. Look at header file comments for detailed explanations
4. Check existing GDScript examples
5. Read the Flecs documentation for core ECS concepts

---

## 📝 Contributing to Documentation

When adding new features:

1. Update relevant `.md` files
2. Add Doxygen comments to headers
3. Include GDScript examples
4. Update this index if adding new docs
5. Keep documentation in sync with code

---

## 🗂️ File Organization

```
flecs_types/
├── README.md                           # Start here
├── QUICK_REFERENCE.md                  # Cheat sheet
├── DOCUMENTATION_INDEX.md              # This file
├── FLECS_SERVER_API.md                 # Complete API
│
├── QUERY_API.md                        # Query guide
├── QUERY_IMPLEMENTATION_README.md      # Query internals
├── QUERY_VS_SCRIPTSYSTEM.md           # Design decisions
│
├── PERFORMANCE_FIX.md                  # Optimization
├── BUILD_SYSTEM_REFACTORING.md        # Code structure
├── CURSOR_CONVERSION_README.md         # Serialization
│
├── flecs_server.h                      # Documented
├── flecs_script_system.h               # Documented
├── flecs_query.h                       # Documented
├── flecs_variant.h                     # Documented
│
└── example_query_usage.gd              # Working examples
```

---

**Documentation Version**: 1.0  
**Last Updated**: 2025  
**Status**: Complete ✅