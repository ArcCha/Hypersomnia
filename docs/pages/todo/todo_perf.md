---
title: ToDo performance
hide_sidebar: true
permalink: todo_perf
summary: Just a hidden scratchpad.
---

- arrange components in such an order that the fundamentals go first, and frequently used go next to each other
	- so that component ids are cached more frequently
- separate rigid body and static rigid body
	- so that the static rigid body does not store velocities and dampings needlessly
	- and so that we only add interpolation component for dynamic rigid bodies
- it is quite possible that step_and_set_new_transforms might become a bottleneck. In this case, it would be beneficial to:
	- 
- for better cache coherency, we might be inclined to store "has_component" array of bools alongside processing ids so that they land in cache and "find" queries are faster. 
	- possibly only optimization for bottlenecks.
- for better cache coherency, we might, inside systematic functions, iterate components and keep track in some cache to which entities they belong
	- so that we always have a perfect coherency at least along a single component 
- statically allocated constexpr flag for entities
- describe concept: quick caches
	- stored directly in the aggregate
	- will be used to store copies of definitions for super quick access
		- e.g. render
	- will be used for super quick inferred caches
		- e.g. if we ever need a transform or sprite cache if they become too costly to calculate (either computationally or because of fetches)
	- will be serialized in binary as well to improve memcpy speed 
		- will need manual rebuild of that data
	- but will be omitted from lua serialization to decrease script size
