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

Verify the link before building:

```sh
test -f Resources/public/foundation.pr
```
