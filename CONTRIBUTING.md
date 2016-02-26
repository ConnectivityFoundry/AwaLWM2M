
![](doc/img.png)

----

## Contributor guide.


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


### Branching model.

All Awa LWM2M development occurs on the master branch. Developers should only submit patches against the master branch.


### Setting up your development machine.

Configure your user name to be used by git:

    $ git config --global user.name "FirstName LastName"
    $ git config --global user.email "email@email.com"


### Coding style.

The Awa LWM2M coding style guidelines can be found in the [Coding style guide](doc/coding_style.md).



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

Awa LWM2M requires contributors to accept the Developer Certificate of Origin (DCO) (from developercertificate.org).

The sign-off is a single line at the end of your commit comment which certifies that you either wrote the supplied code or otherwise have the right to pass on the code as open source.


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
git commit --signoff
````

----

----


