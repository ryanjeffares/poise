if exists('b:current_syntax')
  finish
endif

syn keyword poiseBuiltinFunctions println print eprintln eprint typeof skipwhite
syn keyword poiseBooleans true false skipwhite
syn keyword poiseConditionals and or skipwhite
syn keyword poiseKeywords return struct func var final const import export if else while for in by as this none throw try catch break continue skipwhite
syn keyword poiseType Int Float Bool String None Function Exception List Range Tuple Dict skipwhite

syn keyword poiseTodo TODO FIXME NOTE NOTES XXX contained
syn match poiseComment "//.*$" contains=poiseTodo

syn match poiseNumber "\v<\d+>"
syn match poiseNumber "\v<\d+\.\d+>"
syn region poiseString start='"' end='"'

hi def link poiseTodo               Todo
hi def link poiseComment            Comment
hi def link poiseString             String
hi def link poiseNumber             Number
hi def link poiseBooleans           Boolean
hi def link poiseConditionals       Conditional
hi def link poiseKeywords           Keyword
hi def link poiseBuiltinFunctions   Function
hi def link poiseType               TypeInternal

let b:current_syntax = "poise"
