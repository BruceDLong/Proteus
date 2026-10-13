# Shared public world models

The `public` models are owned by the sibling `World` repository rather than
this repository. The expected checkout layout is:

```text
devl/
├── Proteus/
├── Slipstream/
└── World/
    └── public/
```

From the Proteus repository root, create the local link with:

```sh
ln -s ../../World/public Resources/public
```

Applications and explicit integration runs can verify that link with:

```sh
test -f Resources/public/KernelKB.pr
```


## Test fixtures

Default TestProteus runs use tracked assets in this repository. They do not
require the sibling World repository or a live user knowledge base. Files such
as `user_3.pr`, `userKB.pr` and `rubix_sparse_nested.pr` are committed regression
fixtures; they are not refreshed from application data during a test run.

`TestFiles/world-load/` contains small synthetic kernels and public documents.
The symbolic pair preserves the unresolved `*(20000+_)` world-shape regression.
The finite pair checks successful kernel bootstrap, origin-relative `%INSERT`
from both documents, and a public declaration using a kernel declaration.
The test core resolves a PublicKB fixture's KernelKB in the same directory.
Keep new default fixtures and their transitive imports in tracked test assets.
Actual application-world compatibility remains separate integration work.
