# T-48 — Category Taxonomy Pass (ratified spec + applied mapping)

**Scope:** MaevixCore plugin C++ (`Plugins\MaevixCore\Source\`, both `MaevixCore` and
`MaevixCoreEditor` modules). **Metadata-only:** only editor `UPROPERTY`/`UFUNCTION` `Category=`
display strings were renamed. No identifier/class/function renames, no logic change, no
`CoreRedirects`. Editor `Category=` metadata is stripped from shipping builds and is not serialized
into `.uasset` data, so this cannot affect compilation or break any asset reference.

**Out of scope (untouched):** Part 1's three code fixes; gameplay-tag strings
(`MCore.Settings.Category.*`); `meta=(Categories="…")` tag filters; `LogModulus*`; `[CoreRedirects]`;
all `Content/`; the Modulus→Maevix asset/identifier sweep.

## Applied mapping — 22 distinct strings, 130 occurrences

| # | Old `Category=` string | New `Category=` string | Rule | Occurrences |
|--:|------------------------|------------------------|------|------------:|
| 1 | `UI\|KeyBinding` | `MaevixCore\|UI\|KeyBinding` | orphan `UI\|*` → prepend Tier-1 | 32 |
| 2 | `UI\|KeyBinding\|Input` | `MaevixCore\|UI\|KeyBinding\|Input` | orphan `UI\|*` | 1 |
| 3 | `UI\|Theme` | `MaevixCore\|Theme` | **A** (fold to top-level system) | 7 |
| 4 | `UI\|Settings` | `MaevixCore\|Settings` | **A** (fold to top-level system) | 6 |
| 5 | `UI\|Dialog` | `MaevixCore\|UI\|Dialog` | orphan `UI\|*` | 5 |
| 6 | `UI\|Dialog\|Input` | `MaevixCore\|UI\|Dialog\|Input` | orphan `UI\|*` | 2 |
| 7 | `UI\|HUD` | `MaevixCore\|UI\|HUD` | orphan `UI\|*` | 2 |
| 8 | `UI\|Input` | `MaevixCore\|UI\|Input` | orphan `UI\|*` | 2 |
| 9 | `UI\|Activation` | `MaevixCore\|UI\|Activation` | orphan `UI\|*` | 2 |
| 10 | `UI\|Layout` | `MaevixCore\|UI\|Layout` | orphan `UI\|*` | 1 |
| 11 | `MCore\|UI` | `MaevixCore\|UI` | fix Tier-1 `MCore`→`MaevixCore` | 7 |
| 12 | `MCore\|UI\|MenuHub` | `MaevixCore\|Menu Hub` | **B** (unify to dominant form) | 6 |
| 13 | `MaevixCore\|Input Display` | `MaevixCore\|Input\|Display` | **E** (space→pipe; `Input` Tier-2 exists) | 12 |
| 14 | `MaevixCore\|Network` | `MaevixCore\|Networking` | **C** (Tier-2 → subsystem name) | 5 |
| 15 | `MaevixCore\|Network\|Widget` | `MaevixCore\|Networking\|Widget` | **C** | 5 |
| 16 | `MaevixCore\|Network\|Subsystem` | `MaevixCore\|Networking\|Subsystem` | **C** | 2 |
| 17 | `MaevixCore\|Replication` | `MaevixCore\|Networking\|Replication` | stray top-level → `Networking` | 1 |
| 18 | `MaevixCore\|Network Safety\|Event Validation` | `MaevixCore\|Networking\|Safety\|Event Validation` | **G** | 4 |
| 19 | `MaevixCore\|UI Extension` | `MaevixCore\|UI\|Extension` | **D** (space→pipe under `UI`) | 7 |
| 20 | `Components` | `MaevixCore\|Components` | **F** (no Tier-1 → prepend) | 13 |
| 21 | `Menu Tab` | `MaevixCore\|Menu Hub` | **F** | 4 |
| 22 | `Collection` | `MaevixCore\|Settings\|Collection` | **F** | 4 |
| | | | **TOTAL** | **130** |

### Per-system breakdown (sums to 130)

- **CoreUI — 93** (rows 1–12, 19, 20: KeyBinding/UI/Theme/Settings/Dialog/HUD/Input/Activation/
  Layout/MenuHub/UI-Extension/Components headers under `CoreUI\…`).
- **CoreData — 36** (row 13 Input Display 12; rows 14–16 Networking *library* 12; row 18 Network
  Safety 4; row 21 Menu Tab 4; row 22 Collection 4 — all under `CoreData\…`).
- **CoreNetworking — 1** (row 17, the stray `Replication` on `MCore_NetworkingComponent.h`).
- 93 + 36 + 1 = **130.** ✓

## ClassGroup decision — **OPTION 1: left untouched** (ratified)

`ClassGroup=` is a **flat** editor grouping (class/component picker), not the pipe `Category=`
hierarchy, so its values don't map cleanly onto `MaevixCore|…`. All 19 `ClassGroup` occurrences are
**unchanged**: the 6 `ClassGroup=(MaevixCore)`/`="MaevixCore"` (already Tier-1-correct) and the 13
`ClassGroup="MaevixUI"` (a reasonable flat widgets-vs-components split). Forcing them into the
`Category=` taxonomy would be over-reach; this pass is `Category=` only.

## Notes

- Reconciliation: live source contained **no** `Modulus*`/`ModulusCore|` `Category=` strings — the
  rename was already complete for this metadata. The editor module carries no `Category=` metadata.
- Applied as a byte-preserving in-place replace; the diff is surgical (changed lines = 2× occurrences
  per file; line endings preserved). Post-edit grep confirms zero stale OLD strings.
