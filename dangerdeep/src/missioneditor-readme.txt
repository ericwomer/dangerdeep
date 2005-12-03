The mission editor
------------------

current state: planning

How to do it?
-------------

Maybe in-game, (re)use as many code and graphics as possible.

What should be editable in a first version?  Current missions stored
only a small amount of values.  So a simple mission editor would be
already as good as the current missions.

time

descriptions

convoys (structure)
- add, remove, group
- waypoints
=> convoy speed etc. are determined from waypoints.
   relative positions for ships are determined from
   absolute positions of them when the convoy is created.
   So create ships first, select some of them and select
   "create convoy".

ships, submarines (, airplanes)
- add, remove
- set player
- position, heading, speed (same as throttle for simple version)
- for submarines: some more states (depth, scope state, snorkel state)
  which torpedo types are in the tubes and stored (simple list).
  A random distribution would also be ok for the start.
  Just create objects with random or default values when they are
  created.


