#### CMake-Modules
This repo is a common place to store useful cmake modules, including customized
Find<Package> files and utility modules.  It is intended to be incorporated into projects
as a subtree. 

##### Usage
First add the cmake-module repo as a remote, so you can more easily reference it
```
  git remote add -f cmake-modules-repo git@sf-github.leap.corp:leapmotion/cmake-module.git
```

To setup cmake-modules in your repository (only run once as a setup step):

```
  git subtree add --prefix cmake-modules cmake-modules-repo master --squash
```
To update the copy of cmake-modules in your repository:
```
  git fetch cmake-modules-repo master
  git subtree pull --prefix cmake-modules cmake-modules-repo master --squash
```
To push changes from your repository upstream into the cmake-module repo:
```
  git subtree push --prefix cmake-modules cmake-modules repo <branch>
  Open a pull request to merge <branch> to master
```


For more information on subtrees see Atlassian's [Git Subtree Tutorial](http://blogs.atlassian.com/2013/05/alternatives-to-git-submodule-git-subtree/)


#### Notes & TODOs 

- Better helpers and handling for modules which may be either SHARED or STATIC.

- Proposed solution is to define SHARED_LIBRARY and STATIC_LIBRARY separately, then set
  LIBRARY to whichever one exists, and if both are defined then create an option so the
  user can choose.  Alternatively, we could detect which are available, default to SHARED
  if both are, and expose an Xxx_IMPORT_TYPE cache variable that the user could override.
  This would also let us throw errors if the files for the desired type are unavailable.

- Make a cmake module for installing executables/resources in a platform-agnostic way
  (e.g. Mac has a particular "bundle" format it follows).

- The organization of the Components (with its component and library dependencies) should
  be implemented using a cmake module, so little redundant boilerplate is necessary.
