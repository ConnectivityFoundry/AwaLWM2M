

![](images/img.png)

----

# Extra information for Awa LightweightM2M contributors.


## Rebasing.

Usually rebasing commits that have already been pushed is strongly discouraged. 

The main problem with rebasing pushed commits is that if another contributor has based
their work on one of the commits which is subsequently rebased, this effectively
creates a parallel timeline in which the commits on which their work stands never existed.

Given that we created our own fork, and branched within that fork, it's safe
to assume that nobody will be using it as a baseline. We can therefore rebase without causing
the universe to implode.

That said, here's the process...


First, fetch any upstream changes...

```
$ git fetch upstream 
```

and change to your development branch...

```
$ git checkout dev-branch1
```

### Rebasing your commits on top of upstream/master.

Now if you want to rebase your commits on top without making any additional changes you can use the command:

```
$ git rebase upstream/master
```

### Editing commit messages.

If you want to edit a commit message you will need to use the interactive rebase.

```
$ git rebase -i origin/dev-branch1
```

Your default text editor will launch and display something like...

```
pick d8ff6b0 Update readme
pick 80309d3 Fixed tpyo
pick d38fb35 Deleted line

# Rebase 0bee19a..d38fb35 onto 0bee19a (3 command(s))
#
# Commands:
# p, pick = use commit
# r, reword = use commit, but edit the commit message
# e, edit = use commit, but stop for amending
# s, squash = use commit, but meld into previous commit
# f, fixup = like "squash", but discard this commit's log message
# x, exec = run command (the rest of the line) using shell
# d, drop = remove commit
```
To change a commit comment, select the line you wish to change by replacing 
"pick" with "r" and then saving the document.

```
pick d8ff6b0 Update readme
r 80309d3 Fixed tpyo
pick d38fb35 Deleted line
```

Your text editor will open a new window allowing you to change the commit message...

```
Fixed typo

# Please enter the commit message for your changes. Lines starting
# with '#' will be ignored, and an empty message aborts the commit.
```

Save and quit to apply your changes.


### Squashing commits.

If you want to squash multiple commits together into a single commit you will need to use the interactive
rebase.

```
$ git rebase -i origin/dev-branch1
```

Your default text editor will launch and display something like...

```
pick d8ff6b0 Update readme
pick 80309d3 Fixed tpyo
pick d38fb35 Deleted line

# Rebase 0bee19a..d38fb35 onto 0bee19a (3 command(s))
#
# Commands:
# p, pick = use commit
# r, reword = use commit, but edit the commit message
# e, edit = use commit, but stop for amending
# s, squash = use commit, but meld into previous commit
# f, fixup = like "squash", but discard this commit's log message
# x, exec = run command (the rest of the line) using shell
# d, drop = remove commit
```
To change a commit comment, select the lines you wish to squash by replacing
"pick" with "s" and saving the document.


For example:

```
pick d8ff6b0 Update readme
s 80309d3 Fixed tpyo
s d38fb35 Deleted line
```

If all goes well your text editor will open and display something like... 

```
# This is a combination of 3 commits.
# The first commit's message is:
Update readme

# This is the 2nd commit message:

Fixed typo

# This is the 3rd commit message:

Deleted line
```

Comment out any of the commit messages you no longer require, enter a new message
then save and quit.


###Aborting a rebase.

If you feel uncertain at any stage, you can abort a rebase using the following command:

```
$ git rebase --abort
```


### Pushing your changes.

After rebasing you will not be able to push your changes back to your branch. You will need to use the --force option. 
There's no going back if you make a mistake so be sure about what you are pushing, and that you are pushing the correct branch to the correct remote.

```
$ git push -f origin dev-branch1:dev-branch1
```

The above command will replace origin/dev-branch1 with the contents of dev-branch1, including all history.

----

----

