# Discovering the iPhone input tools

English · [简体中文](tool-discovery.zh-CN.md)

The `0.7.3-discovery` scope is metadata only: add concise English/Chinese task
vocabulary to the seven tool descriptions and a soft recommendation to browse
the whole input group before planning multi-step control. No new tools, schema
changes, direct app-launch API, input execution, grants, transport or client code.

Acceptance defined before this change: parse the embedded tool JSON; retain
names/order, schemas and annotations exactly; preserve executor/HID/radio/config
inputs; limit the server description to 500 characters; keep task words specific
to each tool; retain required timing, ASCII, coordinates, focus and no-replay
constraints; compile/package and check the source/image manifest. Source/static
evidence does not establish model discovery or phone outcomes. No new unit tests
or device/provider execution are part of this scope.

| Tool | Task vocabulary |
| --- | --- |
| `device_status` | Connection, input readiness, physical control, busy state |
| `tap` | Open visible app icons/search results, buttons, links, settings switches |
| `drag` | Swipe/navigate pages, drag objects, draw a stroke or signature |
| `press_key` | Home/Spotlight app-opening workflow, navigation, confirmation, editing, media |
| `type_ascii` | ASCII app names/search queries, URLs, messages and forms in a focused field |
| `scroll` | Browse lists, pages and feeds with wheel input |
| `stop` | Stop/cancel phone input and revoke control |

Opening an app is a client-planned UI workflow: Home or Spotlight with
`press_key`, optional ASCII search text with `type_ascii`, then `tap` on an
observed icon/result. Chinese names cannot be injected by ASCII typing. The
firmware neither accepts bundle IDs nor finds apps, captures screens or reports
app success. Observe before choosing targets and verify afterwards.

The inspected Anan client searches names, descriptions and parameter metadata
with keyword ranking; native Chinese aliases are not added to MCP tools. Its
default five matches are a result-page size, not the server's tool count. For a
complete group overview, use `tool_search` with empty `query`, the exact group
from current client context and `limit: 10`; follow `nextOffset` if needed.
A nonempty query can exclude tools even with a larger limit. Search returns
summaries; `tool_describe` supplies a selected full schema before `tool_call`.
Visible unchanged schemas may be reused. This is guidance, not automatic loading
or permission, and it changes no Anan code.

After flashing this candidate, refresh the client's tool directory. In Anan a
custom service description overrides the server description; update that custom
text separately if needed. Broaden a search restricted to `apple` to include the
MCP group. The client may cache the directory, so a firmware upgrade alone does
not prove the model has received the new descriptions.
