"""Finite scalar numeric infons: a deliberately small, exact data model.

This module does not parse general Proteus syntax (lists, references, ranges,
rational expressions, identity constraints, or ghost-singlet lifts).
"""
from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path
import re
from typing import Any

_INFN = re.compile(r"\s*\*\s*([0-9]+)\s*\+\s*([0-9]+|_)\s*\Z")


@dataclass(frozen=True)
class Infon:
    """A finite *size+value infon; None denotes an unspecified value.

    Values are canonical indices in [0, size).  In particular, an out-of-range
    ghost lift is NOT silently reduced to an ordinary value.
    """

    size: int
    value: int | None = None

    def __post_init__(self) -> None:
        if type(self.size) is not int or self.size < 1:
            raise ValueError("size must be a positive integer")
        if self.value is not None and (
            type(self.value) is not int or not 0 <= self.value < self.size
        ):
            raise ValueError("value must be None or an integer in [0, size)")

    def require_value(self) -> int:
        """Return a known value, rejecting unknown-state arithmetic."""
        if self.value is None:
            raise ValueError(f"{self} has an unknown value")
        return self.value

    def __str__(self) -> str:
        return f"*{self.size}+{self.value if self.value is not None else '_'}"

    @classmethod
    def parse(cls, text: str) -> Infon:
        """Read a single canonical scalar, e.g. '*12+5' or '*12+_'."""
        if not isinstance(text, str):
            raise TypeError("expected a scalar infon string")
        match = _INFN.fullmatch(text)
        if match is None:
            raise ValueError(f"not a scalar infon: {text!r}")
        value = match.group(2)
        return cls(int(match.group(1)), None if value == "_" else int(value))

    def to_dict(self) -> dict[str, int | None]:
        return {"size": self.size, "value": self.value}

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Infon:
        if not isinstance(data, dict) or set(data) != {"size", "value"}:
            raise ValueError("infon data must have exactly 'size' and 'value'")
        return cls(data["size"], data["value"])

    def to_json(self) -> str:
        return json.dumps(self.to_dict(), sort_keys=True)

    @classmethod
    def from_json(cls, text: str) -> Infon:
        return cls.from_dict(json.loads(text))

    def save(self, path: str | Path) -> None:
        Path(path).write_text(self.to_json() + "\n", encoding="utf-8")

    @classmethod
    def load(cls, path: str | Path) -> Infon:
        return cls.from_json(Path(path).read_text(encoding="utf-8"))
