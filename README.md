# Perception Engine

⚡ **Perception Engine** is a high-performance, open-source C++ game engine built from scratch — for learning, experimentation, and fun.

It's designed with a focus on:
- **Clean architecture**
- **Performance-first mindset**
- **Learning by doing**  
No external ECS or game frameworks used. Everything is custom.

---

## 🚀 Roadmap
| Feature                  | Status         |
|--------------------------|----------------|
| Core ECS                  | ✅ Done        |
| Safe GC System            | ✅ Done        |
| Transform Overhaul        | ✅ Done        |
| Rendering Pipeline        | 🚀 In Progress |
| Physics Layer             | 🚀 In Progress |
| C# Scripting Integration  | 🔜 Planned     |
| C++ DLL Plugin Support    | 🔜 Planned     |
| Asset Pipeline            | 🔜 Planned     |

---

## ✅ Current Features

### 🏗️ Core Systems
- **Custom Entity-Component-System (ECS)**
  - Stable memory layout (no archetypes)
  - Parent-child hierarchies with clean attach/detach logic
  - Safe component lifecycle management (create, update, destroy)
  - Interned string pooling for fast lookups & name encryption support (like UE4 FName)

### 🧹 Destruction & GC System
- **Safe destruction queue for entities & components**
  - Multi-thread friendly (frame & physics tick safe)
  - Supports delayed destruction (per-thread tick tracking)
  - Force-deletion for layers/levels (bulk cleanup)
- **Global GC loop running at fixed intervals**

### ⚙️ Transform System Overhaul
- Queued transform updates (position, rotation, scale deltas)
- Dirty flag optimization (only recomputes when needed)
- Parent-relative world transformation with inverse-combine logic

### 🧪 Performance Tested
- ✅ 50,000+ entities with deep hierarchies stress-tested
- ✅ Stable update performance across frame & physics threads

## 💡 Why?
This is a personal journey to understand how modern game engines (like Unreal, Unity) work under the hood — not by reading, but by **building**.

I’m documenting & sharing this project openly to help others who want to learn low-level engine architecture.

---

## 📦 Getting Started
> Currently experimental — best for browsing code & design ideas.  
> Will add build instructions as we progress.

---

## 📝 License
In Processing..

---

## ⭐ Support & Feedback
If you find this interesting, give it a ⭐ star!  
For suggestions, ideas, or just to say hi, open an issue or connect on [LinkedIn](https://www.linkedin.com/in/YOUR-LINKEDIN).

#cpp #gamedev #gameengine #ecs #opensource #learningbydoing #csharp #dllplugins #perceptionengine
