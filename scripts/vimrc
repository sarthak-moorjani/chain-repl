" c/cpp indentation with spaces
autocmd FileType cpp,c,h,cc,python setlocal expandtab shiftwidth=2 tabstop=2 softtabstop=2

""""""""""""""""""""""""""""""""""""""
""""Examples
"Toggle with !
"nmap <f6> :set paste!<cr>

""""""""""""""""""""""""""""""""""""""
function AddTemplate(tmpl_file)
  let fully_qualified_file = expand('%:p')
  if 0 != match(fully_qualified_file, $HOME . '/main/.*')
      return
  endif
    exe "0read " . a:tmpl_file
    let substDict = {}
    let substDict["name"] = $USER
    let substDict["date"] = strftime("%Y %b %d %X")
    exe '%s/<<\([^>]*\)>>/\=substDict[submatch(1)]/g'
    set nomodified
    normal G
endfunction

autocmd BufNewFile *.c,*.cc,*.cpp,*.h,*.go call AddTemplate("~/.vim/template.cpp")

let mapleader = ','

" Colorscheme
set background=dark
if &diff
    colorscheme molokai
  set t_Co=16
endif

" Set 'nocompatible' to ward off unexpected things that your distro might
" have made, as well as sanely reset options when re-sourcing .vimrc
" use vim-defaults instead of vi-defaults (easier, more user friendly)"
set nocompatible

" Map key to toggle opt
function MapToggle(key, opt)
  let cmd = ':set '.a:opt.'! \| set '.a:opt."?\<CR>"
  exec 'nnoremap '.a:key.' '.cmd
  exec 'inoremap '.a:key." \<C-O>".cmd
endfunction
command -nargs=+ MapToggle call MapToggle(<f-args>)

" Attempt to determine the type of a file based on its name and possibly its
" contents. Use this to allow intelligent auto-indenting for each filetype,
" and for plugins that are filetype specific.
filetype plugin indent on

" Enable syntax highlighting
syntax on

" Display the cursor position on the last line of the screen or in the status
" line of a window
set ruler
nnoremap <C-M><C-M> :set invnumber \| ALEToggle \| GitGutterToggle<CR>

" Better command-line completion
set wildmenu

" Show partial commands in the last line of the screen
set showcmd

" Highlight searches (use <C-L> to temporarily turn off highlighting; see the
" mapping of <C-L> below)
set hlsearch

set scrolloff=15
set showcmd             " show typed command in status bar
set showmatch           " show matching bracket (briefly jump)
set smarttab            " smart tab handling for indenting
set fileformat=unix     " file mode is unix
set viminfo='20,\"500   " remember copy registers after quitting in the .viminfo file -- 20 jump links, regs up to 500 lines'
set hidden              " remember undo after quitting

" Uncomment the following to have Vim jump to the last position when
" reopening a file places cursor where it was left off
if has("autocmd")
  au BufReadPost * if line("'\"") > 0 && line("'\"") <= line("$")
    \| exe "normal! g'\"" | endif
endif
"------------------------------------------------------------
" Usability options {{{1
"
" These are options that users frequently set in their .vimrc. Some of them
" change Vim's behaviour in ways which deviate from the true Vi way, but
" which are considered to add usability. Which, if any, of these options to
" use is very much a personal preference, but they are harmless.

" Use case insensitive search, except when using capital letters
set ignorecase
set smartcase

" copy indent from previous line: useful when using tabs for indentation
" and spaces for alignment
set copyindent

" Allow backspacing over autoindent, line breaks and start of insert action
set backspace=indent,eol,start

" When opening a new line and no filetype-specific indenting is enabled, keep
" the same indent as the line you're currently on. Useful for READMEs, etc.
set autoindent

" Stop certain movements from always going to the first character of a line.
" While this behaviour deviates from that of Vi, it does what most users
" coming from other editors would expect.
set nostartofline

" Always display the status line, even if only one window is displayed
set laststatus=2

" Use visual bell instead of beeping when doing something wrong
set visualbell

" And reset the terminal code for the visual bell. If visualbell is set, and
" this line is also included, vim will neither flash nor beep. If visualbell
" is unset, this does nothing.
set t_vb=

" Enable use of the mouse for all modes
"if has('mouse')
"  set mouse=a
"endif

" Set the command window height to 2 lines, to avoid many cases of having to
set cmdheight=2

" Use <F6> to toggle between 'paste' and 'nopaste'
set pastetoggle=<leader>p

" Column width
set textwidth=79
set colorcolumn=+1

" Disable wrap in insert mode when editting a line > textwidth
set formatoptions+=l
" Disable continuation of comments in new line
set formatoptions-=o
set formatoptions-=r

" Highlight trailing spaces
highlight ExtraWhitespace ctermbg=red guibg=red
match ExtraWhitespace /\s\+$/

"MapToggle <f6> paste
"------------------------------------------------------------
" Indentation options {{{1
"
" Indentation settings according to personal preference.

" Indentation settings for using 4 spaces instead of tabs.
" Do not change 'tabstop' from its default value of 8 with this setup.
"set shiftwidth=4
"set softtabstop=4
"set expandtab

" Indentation settings for using hard tabs for indent. Display tabs as
" four characters wide.
" set shiftwidth=2
set tabstop=2

"------------------------------------------------------------
" Mappings

" Map Y to act like D and C, i.e. to yank until EOL, rather than act as yy,
" which is the default
noremap Y y$

" Map <C-L> (redraw screen) to also turn off search highlighting until the
" next search
noremap <C-L> :nohl<CR><C-L>

" Map <C-A> to select all text
noremap <C-a> <esc>gg0vG$<CR>

" Map Ctrl-Backspace to delete the previous word in insert mode. (doesn't work)
inoremap <C-BS> <C-W>

" Select text in visual mode and replace with ctrl+r with confirmation
vnoremap <C-r> "hy:%s/<C-r>h//gc<left><left><left>

"------------------------------------------------------------
set clipboard=unnamedplus,unnamed,autoselect
" Copy a selection to the OSX clipboard [SO] Cut and copy
"vmap <C-x> :!pbcopy<CR>
"vmap <C-c> :w !pbcopy<CR><CR>

" Remapping Esc key
inoremap jk <Esc>
inoremap kj <Esc>
noremap <Home> <Esc>

" Autoclose brackets
function! s:CloseBracket()
    let line = getline('.')
    if line =~# '^\s*\(struct\|class\|enum\) '
        return "{\<Enter>};\<Esc>O"
    elseif searchpair('(', '', ')', 'bmn', '', line('.'))
        " Probably inside a function call. Close it off.
        return "{\<Enter>});\<Esc>O"
    else
        return "{\<Enter>}\<Esc>O"
    endif
endfunction
inoremap <expr> {<Enter> <SID>CloseBracket()

" Moving to panes
"nnoremap <C-J> <C-W><C-J>
"nnoremap <C-K> <C-W><C-K>
"nnoremap <C-L> <C-W><C-L>
"nnoremap <C-H> <C-W><C-H>

set splitbelow
set splitright

nnoremap <C-w>< :vertical res-15<CR>
nnoremap <C-w>> :vertical res+15<CR>
nnoremap <C-w>- :res-15<CR>
nnoremap <C-w>+ :res+15<CR>

" Vim autoupdates file if changed in other place
set autoread
" --------------------------------------------------------------
" Setup

" tagbar: go get -u github.com/jstemmer/gotags

"call plug#begin('~/.vim/plugged')
"Plug 'fatih/vim-go', { 'do': ':GoUpdateBinaries' }
"Plug 'vim-airline/vim-airline'
"Plug 'tpope/vim-sensible'
"Plug 'ervandew/supertab'
"Plug 'airblade/vim-gitgutter'
"Plug 'majutsushi/tagbar'
"Plug 'dense-analysis/ale'
"Plug 'will133/vim-dirdiff'
"Plug 'tpope/vim-sleuth'
"call plug#end()

" vim-gitgutter
"<leader>hp : Hunk preview (default)

" supertag
set completeopt=menu,longest    " Use the popup menu by default; only insert the longest common text of the completion matches; don't automatically show extra information in the preview window.

" tagbar

nmap <leader>t :TagbarToggle<CR>
let g:tagbar_type_go = {
  \ 'ctagstype' : 'go',
  \ 'kinds'     : [
    \ 'p:package',
    \ 'i:imports:1',
    \ 'c:constants',
    \ 'v:variables',
    \ 't:types',
    \ 'n:interfaces',
    \ 'w:fields',
    \ 'e:embedded',
    \ 'm:methods',
    \ 'r:constructor',
    \ 'f:functions'
  \ ],
  \ 'sro' : '.',
  \ 'kind2scope' : {
    \ 't' : 'ctype',
    \ 'n' : 'ntype'
  \ },
  \ 'scope2kind' : {
    \ 'ctype' : 't',
    \ 'ntype' : 'n'
  \ },
  \ 'ctagsbin'  : 'gotags',
  \ 'ctagsargs' : '-sort -silent'
\ }
let g:tagbar_sort = 0

" vim-go
let g:go_fmt_command = "goimports"

" ale
let g:ale_cpp_gcc_options = '-Wall'
"let g:ale_linters = {'c': ['gcc'], 'cpp': ['g++']}
let g:ale_linters = {
  \ 'go': ['gopls'],
  \}

