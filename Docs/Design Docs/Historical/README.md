# Historical design notes

These four documents came from theory/ and remain as earlier design references:

- [Negative-index time design](negative-index-time-design.md).
- [Negative-index implementation guide](negative-index-time-implementation-guide.md).
- [Infon operation catalog](infon-operation-catalog.md).
- [Normalization mutation matrix](normalization-mutation-matrix.md).

Use [the current design](../normalization-design.md) and
[the decision log](../normalization-design-decisions.md) for the maintained
contract. Older APIs, syntax and implementation details describe their original
checkpoint.

## Recover removed graph records

The graph/ directory and the 78 Markdown reports moved from it into this folder
were removed from the checkout at the user's request. They are preserved in
commit `5dbbe10`, immediately before that cleanup. References marked as Git
history in the remaining design documents refer to these saved files.

From the repository root, list the saved files or read one without restoring it:

```sh
git ls-tree -r --name-only 5dbbe10 -- graph 'Docs/Design Docs/Historical'
git show 5dbbe10:graph/engine-test-migration.json
git show '5dbbe10:Docs/Design Docs/Historical/normalization-framework-audit.md'
```

For a reference written as `Historical/<name>.md` in a design document, use
`Docs/Design Docs/Historical/<name>.md` as the path after the commit's colon.
The active language fixtures remain in Tests/; their contents were not removed.
