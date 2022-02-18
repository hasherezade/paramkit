# ParamKit
[![Build status](https://ci.appveyor.com/api/projects/status/dw7xwgd9isgvsair?svg=true)](https://ci.appveyor.com/project/hasherezade/paramkit)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/eb0766850dbf4a389b54aa04ae92906d)](https://app.codacy.com/gh/hasherezade/paramkit/dashboard?branch=master)
[![Commit activity](https://img.shields.io/github/commit-activity/m/hasherezade/paramkit)](https://github.com/hasherezade/paramkit/commits)
[![Last Commit](https://img.shields.io/github/last-commit/hasherezade/paramkit/master)](https://github.com/hasherezade/paramkit/commits)

A small library helping to parse commandline parameters (for Windows).

Objectives
-
+  "like Python's `argparse` but for C/C++"
+  compact and minimalistic
+  easy to use
+  extendable

You can quickly create a project using ParamKit basing on the given template:

+   [Project template](https://github.com/hasherezade/paramkit_tpl)

Docs
-
📚 [https://hasherezade.github.io/paramkit/](https://hasherezade.github.io/paramkit/)

Demo
-

Print help for each parameter:

<img src="https://raw.githubusercontent.com/hasherezade/paramkit/master/img/demo_help.png" alt="demo: print help" >

Paramkit allows you to cluster your parameters into custom groups. 

You can use predefined, popular parameter types, or add your custom ones.

Easily parse and store values, and verify if all required parameters are filled:

<img src="https://raw.githubusercontent.com/hasherezade/paramkit/master/img/demo_print.png" alt="demo: print filled params" >

Verify if no invalid parameter was passed, and eventually suggest similar parameters:

<img src="https://raw.githubusercontent.com/hasherezade/paramkit/master/img/demo_invalid_param.png" alt="demo: print invalid param" >

See [the demo code](https://github.com/hasherezade/paramkit/tree/master/demo)
