# vim plugin
```
sudo npm -g install instant-markdown-d
git clone https://github.com/suan/vim-instant-markdown
cd vim-instant-markdown/
mkdir -p ~/.vim/after/ftplugin/markdown/
cp after/ftplugin/markdown/instant-markdown.vim ~/.vim/after/ftplugin/markdown/
```
Add the following line in ~/.vimrc
```
autocmd BufNewFile,BufFilePre,BufRead *.md set filetype=markdown
```
