# Contributing to uwufetch

## Code

To contribute to this project, you should follow some rules to keep the code consistent:
- To indent I like to use tabs in size 4, so you should use them to commit.

- When the code in an `if` (or `else`) statement is only one line please do not use curly brackets: 
```c
if (things)
	something(will_happen);

else if (things < 5)
	something(NULL);

else
	something(!will_happen);
```

- When the code in an `if` (or `else`) statement is too short, write it in one line (if it is readable):
```c
if (this) that();

else if (that) this();

else nothing();
```

- Function and variable names should be written in snake_case and abbreviated if too long.
- Use shell commands only if necessary, just to improve `uwufetch` speed.
- Be sure to reset [debug](https://github.com/TheDarkBug/uwufetch/blob/8205a8cad7e728628a26441969911d5f384132d7/Makefile#L14) in [Makefile](https://github.com/TheDarkBug/uwufetch/blob/main/Makefile) if you edited it.
- Before pushing the commit, please delete double-new-lines.

#

## Pull requests

Before sending a pull request be sure that no one is already working on the same thing and to follow this guide-lines.

With pull requests you can `[FIX]` a bug (reported or not), add `[OS-SUPPORT]`, add a `[NEW-FEATURE]` requested in an [issue](https://github.com/TheDarkBug/uwufetch/blob/main/CONTRIBUTING.md#issues), fix a `[TYPO]` or `[OPTIMIZE]` the code. For everything else do not use tags.

#

## Issues

You can use the issues to report bugs with `[BUG]`, to request features `[FEATURE-REQUEST]`, to request support for an os `[OS-SUPPORT]`. For everything else do not use tags.

If you are reporting a `[BUG]`, please include a screenshot and the output of the command (if a command is used in `uwufetch`).

If you are requesting a feature, please specify if you are already working on it, then send a [pull request](https://github.com/TheDarkBug/uwufetch/blob/main/CONTRIBUTING.md#pull-requests).

#

## Conclusions

I know that adding this file now is a bit late, but I am writing this anyway, just to appear as a *professional* programmer, even though I am not.

I would take some space to thank all the [contributors](https://github.com/TheDarkBug/uwufetch/graphs/contributors) that made this project better every day.
