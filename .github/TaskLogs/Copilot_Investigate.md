# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

## Task 1

This task happens in `VlppOS` repo.

`IChannelClient::OnError` is renamed to `IChannelClient::OnReadError`. This is a callback that only listen to `IChannelServer::BroadcastError`.
`IChannelClient::OnLocalError` is added. Other errors go here.
- For example, currently `HttpClient` calls `RaiseErrorUnsafe` when there is any connection error happens, it should call `OnLocalError` instead.
- When `/Connect` or `/Respond` fails, retry for 3 times. Each failure calls `OnLocalError`, non fatal except for the last retry
- When `/Request` fails, just retry, because it would timeout when the http server does not send any request, no `OnLocalError` is needed.
- When the client already stopped or disconnected, no retry is issued, and in this case, no `OnLocalError` is needed.
- For named pipe, any error causing the pipe to close would issue `OnLocalError` with fatal.
- Only after calling `OnLocalError` with fatal, disconnect the client.
`HttpServerApi::SendResponse` should group the last 4 arguments into a struct `HttpServerResponse`.

## Task 2

This task happens in `VlppParser2` repo.
Release `VlppOS` to `VlppParser2`.

Move `JsonNodeListSerializer` from `GuiRemoteProtocol_Channel_Json.h` in `GacUI` to `GlrJson.h` in `VlppParser2`, put it in namespace `vl::glr::json`.
Add a similar `XmlElementListSerializer` in `GlrXml.h`.
- When serializing, group all elements in an `<Array>` element, build an element (not document) and `XmlPrint`.
- When deserializing, just use all elements in the root element, no need to care about the name or attributes of the root element. Use `XmlParseElement`.

## Task (Final Validation)

This task happens in `GacUI` and `GacJS` repo.
Release `VlppOS` and `VlppParser2 to `GacUI`.

### GacUI

Make sure `RemotingTest_Core` and `RemotingTest_Win32_Renderer` works with `/Http /FCT`.
- Since `IChannelClient::OnError` is renamed, the renderer will build break, just rename the function too, but no need to handle `OnLocalError`.
- Delete `JsonNodeListSerializer` as it is moved to `VlppParser2`.

To verify that, you should launch both processes with the debugger, so that you are able to know the renderer actually communicate commands correctly with core.
You may need to write a piece of temporary powershell script to close the process in a gentle way:
  - You can use whatever way you like, including calling Windows API, to close the renderer gently.
  - After the renderer exits, ensure that core will be notified and exits. If the code has no problem core should already been working that way.
  - Delete that script before commiting.

Release `GacUI`.

### GacJS

Follows `Tools\DebugGacUIWithBrowser.md` to run `RemotingTest_Core` with `/Http /RPT` and make sure `GacJS` could launch and operate the UI.
Make sure test cases work. Half of tests fail in `Gaclib\website\entry`, figure out why and fix all of them.
- Hint: it works on an en-US machine which is faster.
- You are now running on an zh-CN machine which is slower.
- Some cases fail because of localization, make sure it works on both zh-CN and en-US.
- Machine performance might not be a factor, make your own judgement.

# UPDATES

# TEST [CONFIRMED]

Use generated parser coverage and downstream import verification:

- Copy `VlppOS\Release\VlppOS.*` into `VlppParser2\Import` without `IncludeOnly`.
- Build `Test\UnitTest\UnitTest.sln` after adding serializers because `GlrJson.h` and `GlrXml.h` are public headers.
- Run the required Parser2 unit projects in order if the full suite is practical; at minimum run the JSON and XML built-in tests that exercise parsing/printing.
- Success criteria: `JsonNodeListSerializer` and `XmlElementListSerializer` compile from `VlppParser2`, imported VlppOS headers expose `OnReadError`/`OnLocalError`, and selected parser tests pass without memory leak output.

Confirmed with:

- `& C:\Code\VczhLibraries\VlppParser2\.github\Scripts\copilotBuild.ps1` from `Test\UnitTest`: build succeeded with 0 errors and one existing linker warning.
- `ParserTest_AstGen`, `ParserTest_AstParserGen`, `ParserTest_LexerAndParser`, `ParserTest_LexerAndParser_Generated`, `ParserTest_ParserGen`, `ParserTest_ParserGen_Compiler`, `ParserTest_ParserGen_Generated`, `BuiltInTest_Compiler`, `BuiltInTest_Json`, `BuiltInTest_Xml`, `BuiltInTest_Workflow`, and `BuiltInTest_Cpp` all completed through `copilotExecute.ps1`.
- Required rebuilds after generator-style projects completed through `copilotBuild.ps1`.
- `Test\TypeScript\prepare.ps1; npm run build` succeeded with TypeScript `tsc --noEmit`.
- `Tools\Tools\CodePack.backup.exe Release\CodegenConfig.xml` regenerated `VlppGlrParser.*`.

# PROPOSALS [CONFIRMED]

- No.1 Release VlppOS and host JSON/XML list serializers in Parser2

## No.1 Release VlppOS and host JSON/XML list serializers in Parser2

Refresh `VlppParser2\Import` from the updated `VlppOS\Release` files, move the GacUI JSON node list serializer into `vl::glr::json`, and add an XML element list serializer in `vl::glr::xml`. The XML serializer creates an `<Array>` element, appends all source elements as subnodes, and serializes that element with `XmlPrint`; deserialization parses an element with `XmlParseElement` and copies all root sub elements to the destination list.

### CODE CHANGE

- Copied updated `VlppOS\Release` files into `VlppParser2\Import` so Parser2 sees `IChannelClient::OnReadError`, `IChannelClient::OnLocalError`, and the `HttpServerResponse` signature.
- Added `vl::glr::json::JsonNodeListSerializer` to `GlrJson.h/.cpp`, preserving the existing GacUI serialization behavior for JSON node lists.
- Added `vl::glr::xml::XmlElementListSerializer` to `GlrXml.h/.cpp`, serializing element lists under an `<Array>` element with `XmlPrint` and deserializing all child elements of the parsed root.
- Regenerated `Release\VlppGlrParser.h` and `Release\VlppGlrParser.cpp` with CodePack.

### CONFIRMED

The Parser2 source/import/release changes compile and pass the required parser test sequence. The TypeScript target also builds after preparing generated TypeScript test files.
