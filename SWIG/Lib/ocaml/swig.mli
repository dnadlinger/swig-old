(* -*- tuareg -*- *)
type 'a c_obj_t = 
    C_void
  | C_bool of bool
  | C_char of char
  | C_uchar of char
  | C_short of int
  | C_ushort of int
  | C_int of int
  | C_uint of int32
  | C_int32 of int32
  | C_int64 of int64
  | C_float of float
  | C_double of float
  | C_ptr of int64 * int64
  | C_array of 'a c_obj_t array
  | C_list of 'a c_obj_t list
  | C_obj of (string -> 'a c_obj_t -> 'a c_obj_t)
  | C_string of string
  | C_enum of 'a
  | C_director_core of 'a c_obj_t * 'a c_obj_t option ref

type empty_enum = [ `SWIGFake | `Int of int ]

exception InvalidDirectorCall of empty_enum c_obj_t

val invoke : 'a c_obj_t -> (string -> 'a c_obj_t -> 'a c_obj_t)
val convert_c_obj : 'a c_obj_t -> 'b c_obj_t
val fnhelper : bool -> ('a c_obj_t list -> 'a c_obj_t list) -> 'a c_obj_t -> 'a c_obj_t

val get_int : 'a c_obj_t -> int
val get_float : 'a c_obj_t -> float
val get_string : 'a c_obj_t -> string
val get_char : 'a c_obj_t -> char
val get_bool : 'a c_obj_t -> bool

val make_float : float -> 'a c_obj_t
val make_double : float -> 'a c_obj_t
val make_string : string -> 'a c_obj_t
val make_bool : bool -> 'a c_obj_t
val make_char : char -> 'a c_obj_t
val make_char_i : int -> 'a c_obj_t
val make_uchar : char -> 'a c_obj_t
val make_uchar_i : int -> 'a c_obj_t
val make_short : int -> 'a c_obj_t
val make_ushort : int -> 'a c_obj_t
val make_int : int -> 'a c_obj_t
val make_uint : int -> 'a c_obj_t
val make_int32 : int -> 'a c_obj_t
val make_int64 : int -> 'a c_obj_t

val new_derived_object: 
  ('a c_obj_t -> 'a c_obj_t) ->
  ('a c_obj_t -> string -> 'a c_obj_t -> 'a c_obj_t) ->
  'a c_obj_t -> 'a c_obj_t
  
