
![](doc/img.png)

----

# Awa LightweightM2M contributor guide.


## Reporting issues and bugs.

If you discover a bug, or find an issue or area that you feel needs improvement:

* Navigate to the "Issues" tab on the project github page https://github.com/FlowM2M/AwaLWM2M/issues  

* Click the "New Issue" button.  


When making your report, be as clear and concise as possible. Use the following list as a guide:



* Describe the issue, and why you believe it's a problem. 

* Include the Awa LWM2M version or commit id.

* Describe environment details (OS/distribution etc).

* State how often the issue has occurred.

* Describe the steps to take to uncover or reproduce the issue (if any).

* Expected behaviour.

* Actual behaviour.

* Any additional infomation, log output etc.


----


## Developer submissions.

### Create a git hub account.

Before you can contribute you require a git hub account.

Please sign up for an account at https://github.com/

### Add your ssh key to your git hub account.

If you are using a Linux development environment, you can normally find your public key (if you have generated one) in ~/.ssh/id_rsa
or alternatively you can generate a new key but using the following commands:

```
ssh-keygen -t rsa -b 4096 -C "user.name@email.com"
````
When you're prompted to "Enter a file in which to save the key," press Enter. This accepts the default file location.

Now that you have a key, sign in to your git hub account

click the top right icon, select settings->ssh keys->new ssh key

paste the contents of your public key into the box.

### Set your git user name.

It is important that you set the user name and email address to use for any commits to the 
git repository.

```
$ git config --global user.name User Name
$ git config --global user.email User.Name@email.com
```

use the same email address as the one you used to sign up for your github account.

### Forking the repository.

If you want to contribute to the project the best practice is to create a fork. To do so
navigate to https://github.com/FlowM2M/AwaLWM2M and click on the "fork" button at the top right of the 
screen.  If you are a member of multiple organisations you will be presented with a selection screen
which can be used to select where to create the fork. click on your user account to create the fork.

you can now clone your fork 

```
$ git clone git@github.com:<username>/AwaLWM2M.git
```

### Keeping your fork in sync.

In order to easily pull down upstream changes to your fork you need to setup a new remote.

```
$ git remote add upstream git@github.com/FlowM2M/AwaLWM2M.git
```

you can now fetch from the upstream repository with:

```
$ git fetch upstream
```

and merge any new changes into your master branch with:

```
$ git merge upstream/master
```

### Making changes.

We have found that the easiest way of working is to keep a clean master branch in the new fork
and create branches for each new pull request. This prevents merge conflicts with the upstream
master branch, and allows you to make changes to your pull request if required.

To create a new branch

```
$ git checkout master -b dev-branch1 --track
```

*NOTE: the branch name here is important, it will show up in the git history, so it is recommended
to use something meaningful.*

Once you have created your branch make any changes you desire, then commit them to your new branch.

### Coding style.

The Awa LWM2M coding style guidelines can be found in the [coding style guide](doc/coding_style.md).

### Commit messages.

For the commit message, the following rules apply:

 * The first line should be a brief summary of the patch.
 * Leave a blank line after the summary.
 * Provide a detailed description of the change.
 * Leave a blank line after the description.
 * If the patch relates to an issue, add a line with 'Ref: ISSUE_ID'.
 * Add a Gerrit 'Change-id' line.
 * Add a Git 'Signed-off-by' line using ````git commit -s```` or ````git commit --sign-off```` (see *Signing your work* below).

 Example:

````

    Adds a new example feature xyz

    This patch adds example feature xyz. This feature merely acts
    as an example of how to commit something to the project.
    For real features this would contain some text
    describing in detail what the new feature actually does.

    Ref: AWA-2131
    Change-Id: Id564ab1230913abf88123dff193b1231b1
    Signed-off-by: User Name <user.name@email.com>

````



### Signing your work.

Awa LWM2M requires contributors to accept the Developer Certificate of Origin (DCO) from developercertificate.org.

The sign-off is a single line at the end of your commit comment to certify that you either wrote the supplied code or otherwise have the right to pass on the code as open source.


Certifying your contribution asserts that *for the current submission* the following statement is true:



```
Developer Certificate of Origin
Version 1.1

Copyright (C) 2004, 2006 The Linux Foundation and its contributors.
660 York Street, Suite 102,
San Francisco, CA 94110 USA

Everyone is permitted to copy and distribute verbatim copies of this
license document, but changing it is not allowed.

Developer's Certificate of Origin 1.1

By making a contribution to this project, I certify that:

(a) The contribution was created in whole or in part by me and I
    have the right to submit it under the open source license
    indicated in the file; or

(b) The contribution is based upon previous work that, to the best
    of my knowledge, is covered under an appropriate open source
    license and I have the right under that license to submit that
    work with modifications, whether created in whole or in part
    by me, under the same open source license (unless I am
    permitted to submit under a different license), as indicated
    in the file; or

(c) The contribution was provided directly to me by some other
    person who certified (a), (b) or (c) and I have not modified
    it.

(d) I understand and agree that this project and the contribution
    are public and that a record of the contribution (including all
    personal information I submit with it, including my sign-off) is
    maintained indefinitely and may be redistributed consistent with
    this project or the open source license(s) involved.
```

To certify your submission just add the following line to *every* git commit message to indicate that you accept the above DCO:

````
Signed-off-by: User Name <user.name@email.com>

````

If you set-up your user.name and user.email via git config, you can sign your commit automatically with git commit:

````
$ git commit --signoff
````

### Pushing your changes to your branch.

If you specified --track when you created your new branch you should be able to simply push using

```
$ git push
```

If not you will either have to specify where to push your new commits.

```
$ git push origin dev_branch1:dev_branch1
```

or alternatively setup branch tracking 

```
$ git push --set-upstream origin dev_branch1
```

### Creating a pull request.

- Navigate to https://github.com/<username>/AwaLWM2M/pulls
- click on "new pull request"
- select FlowM2M/AwaLWM2M/master as the base in the left hand box
- select the branch you wish to submit as a pull request in the right hand box.
- click the create button.

*NOTE: All Awa LWM2M development occurs on the master branch. Developers should only submit patches against the master branch.*

An email will be sent to the project maintainers, who will review your pull request.

If everything checks out no further action will be required.

You may wish to continue making other changes, in this case simply resync with the upstream
and create a new branch. Do NOT add your new unrelated changes to the branch you 
used for the pull request as they will automatically be included in the request.

### Making changes to a pull request.

You may be asked to make some changes before your pull request will be accepted.
Any further commits that are pushed to the branch you used for the initial pull 
request will be automatically added to your pull request.

In some cases you may be asked to re-base your commits, either to bring them in-line
with the current master branch, to tidy up any commit comments or to add a forgotten 
sign-off. You can find more information on rebasing in the section below.

#### Rebasing.

In most cases, rebasing commits that have already been pushed is considered a big no no. 

The main problem is rebasing pushed commits is that if someone else happens to have based
their work off one of these commits and then you go and rebase it, then you have effectively
created a parallel universe where the commits they based their work on never existed..

Given that we created our own fork, and branch within that fork, it should be reasonably safe
to assume nobody else is using this work as a baseline, allowing us to rebase without causing
the universe to implode.

with that said, lets get started.

firstly fetch any upstream changes:

```
$ git fetch upstream 
```

Change to your development branch, assuming you haven't already

```
$ git checkout dev-branch1
```

##### Rebasing your commits on top of upstream/master.

Now if you want to rebase your commits ontop without making any addition changes
you can simply use the command:

```
$ git rebase upstream/master
```

##### Editing commit messages.

If you want to edit a commit messages you will need to use the interactive
rebase.

```
$ git rebase -i origin/dev-branch1
```

Your default text editor will launch and display something like

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
To simply change a commit comment, select the line you wish to change by replacing 
"pick" with "r" and then saving the document.

```
pick d8ff6b0 Update readme
r 80309d3 Fixed tpyo
pick d38fb35 Deleted line
```

Your text editor will open a new window allowing you to change the commit message

```
Fixed typo

# Please enter the commit message for your changes. Lines starting
# with '#' will be ignored, and an empty message aborts the commit.
```

Save and quit to apply the changes.

##### Squashing commits.

If you want to squash multiple commits together into a single commit you will need to use the interactive
rebase.

```
$ git rebase -i origin/dev-branch1
```

Your default text editor will launch and display something like

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
To simply change a commit comment, select the lines you wish to squash by replacing
"pick" with "s" and saving the document.

for example:

```
pick d8ff6b0 Update readme
s 80309d3 Fixed tpyo
s d38fb35 Deleted line
```

If all goes well your text editor will open and display something similar to 

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

##### Aborting a rebase.

If you ever feel like you have messed up, you can abort a rebase using the following command:

```
$ git rebase --abort
```

##### Pushing your changes.

After rebasing you will no able to simply push you changes back to your branch. You will need to use the --force
option. becareful here, there is no going back if you make a mistake, so be sure to make sure you are happy with 
what you are pushing and that you are pushing the correct branch to the correct remote.

```
$ git push -f origin dev-branch1:dev-branch1
```

This command will replace origin/dev-branch1 with the contents of dev-branch1, including all history.


----

----


