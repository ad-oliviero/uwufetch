# Contributing to uwufetch
First of all, a big thank you to all the [contributors](https://github.com/TheDarkBug/uwufetch/graphs/contributors) that make this project better every day.

If you want to be one of them, please read this contributor guide before contributing to the project.

`uwufetch` and `libfetch` are distributed under the [GPLv3](./LICENSE) license, so you can modify it to your liking.

I should make it clear that I am **not** (yet) a professional developer, so I could be wrong on something. Please correct me if needed.

## Generic code rules
- You must use `clang-format` (default configuration in [.clang-format](./.clang-format)) before committing
- Function and variable names should be written in `snake_case` and abbreviated if too long
- Use plain `C` (if available) instead of `shell` called by `C` functions
- The `C` standard used in this project is `gnu18`
- You should test your code on the platform it was developed on, by either:
  - if the new code is part of `libfetch`, write appropriate tests and run them
  - if the new code is part of `uwufetch`, you can just run it with `uwufetch -c ./default.config`
- Any external dependances must be added to [README.md](https://github.com/TheDarkBug/uwufetch#requisites).

### Writing `get_.*` functions (`libfetch`)
The following code is a template to write a `get_.*` function for `libfetch`:

```c
<type> get_<thing>(void) {
    <type> <thing_variable>;
#if defined(SYSTEM_BASE_LINUX)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_ANDROID)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_FREEBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_OPENBSD)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_MACOS)
  LOG_E("Not implemented");
#elif defined(SYSTEM_BASE_WINDOWS)
  LOG_E("Not implemented");
#else
  LOG_E("System not supported or system base not specified");
#endif
  LOG_V(thing_variable);
  return thing_variable;
}
```
#### Notes:
- Two "system base" sections (`#if defined(SYSTEM_BASE_**)`) can be joined if the code is **exactly the same**
- Of course you have to remove `LOG_E("Not implemented");` if the function is implemented for that system base

## Pull requests
Before sending a pull request, check that no one is already working on the same thing and to follow these guide-lines.

With pull requests you can:
- Fix **[FIX]** to a bug (reported or not)
- Implement **[OS-SUPPORT]**
- Implement a **[NEW-FEATURE]** requested in an [issue](https://github.com/TheDarkBug/uwufetch/blob/main/CONTRIBUTING.md#issues) (or not)
- Fix a **[TYPO]**
- Implement **[OPTIMIZATION**(s)**]**

After implementing the new code, you should add documentation to:
- [uwufetch.1](./uwufetch.1)
- [README.md](./README.md)
- [The wiki](https://github.com/TheDarkBug/uwufetch/wiki) (not yet actually, but It will be available soon)

## Issues
Before writing an issue, check that no one has already had the same issue.

With issues you can:
- Report a **[BUG]**
- Request **[OS-SUPPORT]**
- Request a **[NEW-FEATURE]**
