
![](doc/images/img.png)

----

# Awa LightweightM2M contributor guide.


## Reporting issues and bugs.

If you discover a bug, or find an issue or area that you feel needs improvement:

* Navigate to the "Issues" tab on the project Github page https://github.com/FlowM2M/AwaLWM2M/issues  
* Click the "New Issue" button.  


When making your report, be as clear and concise as possible. Use the following list as a guide:

* Describe the issue, and why you believe it's a problem. 
* Include the Awa LightweightM2M version or commit id.  
* Describe environment details (OS/distribution etc).  
* State how often the issue has occurred.  
* Describe the steps to take to uncover or reproduce the issue (if any).  
* Expected behaviour.  
* Actual behaviour.  
* Any additional infomation, log output etc.  


----


## Developer submissions.

### Create a Github account.

Before you can contribute you require a Github account.

You can sign up for a free account here: [https://github.com/](https://github.com/)

### Add your ssh key to your Github account.

If you are using a Linux development environment, you can normally find your public key in *~/.ssh/id_rsa.pub*. Alternatively you can generate a new key with the following command:

```
ssh-keygen -t rsa -b 4096 -C "user.name@email.com"
````

When you're prompted to enter a file in which to save the key, press Enter to accept the default file location.

Now that you have a key:
* Sign in to your Github account
* Click the top right icon  
* Select *settings -> ssh keys -> new ssh key*
* Paste the contents of your public key into the box.

Your public key should look something like:
```
ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQDOvXIAom7iVB/JmNFZrDNZVz+ta6vZoAvoyzjAR53
LwFyA82TDK4RkosKZHgEU/KT+AXYZ9983uVvzS/O7rxa1YxiL21ckw8Ymm4qRxjMP6Bvw8vsGXlLfq7
bNH2tmxIMxd/csIR246FxmCIddLcrIJ2JOTF3AXNRX8uw0FFeJuZTIkAF/PLDO4HhStY6AGxDzpgoZt
480EdPXzNRqTPJ41iXmMZhsJ3I7HCNeZHmy4VFk0XWKXdvmYKm7nSHeqMZxA9LHRPbLnolJyuBp7qPJ
yWC8xT48dS8PDmn5i/wYyXEBNS6uwsYfLZuPfPAKaSTzd1g1fphEw4/9rTZIqqUSMBR87IdDhVKffvf
tQD4O0TTJAsVEzx6w2dx29lwKWrhuJ9ipSZyH+ujai+azW52b+RmcFOqXh7E79XvWfMF1NqiEDtlqFV
gqnvYg+PkS4+sCyLE0qfMx301r9E6pSTj0SIsBz0PUHFhNUx3grg4eJ1FH8Zu9+JYaFjZeNEFSypxaS
Z7J6coWx8hfBG6bpdnFsuL8JJHKwzlU2OdUJ78uLGgl190OYOEsIz5k3yK49nTyky4sOGmJsmds+fqR
rbURhDGu/tLAAK/6np3fai5ef4beZDbdhXrWS+rjOKSU0lRifUXU/JJFicG2PiX2B1InuqGLwGwAp/3
xoHqsuyNhTw== user.name@email.com

```

### Set your git user name.

It is important that you set the user name and email address to use for any commits to the 
git repository:

```
$ git config --global user.name User Name
$ git config --global user.email User.Name@email.com
```

Use the same email address that you used to sign up for your github account.

### Forking the repository.

If you want to contribute to the project the best practice is to create a fork. To do so
navigate to *https://github.com/FlowM2M/AwaLWM2M* and click on the *fork* button at the top right of the 
screen. If you are a member of multiple organisations you will be presented with a selection screen
which can be used to select where to create the fork. Click on your user account to create the fork.

You can now clone your fork: 

```
$ git clone git@github.com:<username>/AwaLWM2M.git
```

### Keeping your fork in sync.

In order to easily pull down upstream changes to your fork you need to setup a new remote.

```
$ git remote add upstream git@github.com/FlowM2M/AwaLWM2M.git
```

You can now fetch from the upstream repository with:

```
$ git fetch upstream
```

And merge any new changes into your master branch with:

```
$ git checkout master
$ git merge upstream/master
```

### Making changes.

The simplest way of working is to keep a clean master branch in the new fork and to create branches for each new pull request. This prevents merge conflicts with the upstream
master branch, and allows you to make changes to your pull request if required.

To create a new branch:

```
$ git checkout master -b dev-branch1 --track
```

**Note.** *The branch name here is important. It will show up in the git history so use something meaningful or suitably general*

Once you have created your branch make your changes, then commit them to your new branch.

### Coding style.

The Awa LightweightM2M coding style guidelines can be found in the [coding style guide](doc/coding_style.md).

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

Awa LightweightM2M requires contributors to accept the Developer Certificate of Origin (DCO) from developercertificate.org.

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

- Navigate to https://github.com/username/AwaLWM2M/pulls
- Click on *New pull request*
- Select FlowM2M/AwaLWM2M/master as the base in the left hand box
- Select the branch you wish to submit as a pull request in the right hand box.
- Click the *create* button.

**Note.** *All Awa LightweightM2M development occurs on the master branch. Developers should only submit patches against the master branch.*

An email will be sent to the project maintainers who will review your pull request.

If everything checks out no further action will be required.

You may wish to continue making other changes, in this case simply resync with the upstream
and create a new branch. Do **NOT** add your new unrelated changes to the branch you 
used for the pull request as they will automatically be included in the request.

### Making changes to a pull request.

You may be asked to make some changes before your pull request will be accepted.
Any further commits that are pushed to the branch you used for the initial pull 
request will be automatically added to your pull request.

In some cases you may be asked to rebase your commits, either to bring them in-line
with the current master branch, to tidy up any commit comments or to add a forgotten 
sign-off. You can find more information on rebasing [here](doc/rebasing_info.md).


----

----




