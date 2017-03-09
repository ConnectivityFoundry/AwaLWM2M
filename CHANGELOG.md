# Change Log

## [0.2.5](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.5) (2017-02-03)

[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.2.4...0.2.5)

**Implemented enhancements:**

- \[PROPOSAL\] - Add configuration file for daemons [\#314](https://github.com/FlowM2M/AwaLWM2M/issues/314)
- Add release build to CMake [\#332](https://github.com/FlowM2M/AwaLWM2M/pull/332) ([datachi7d](https://github.com/datachi7d))
- Various fixes for compiler errors and some build improvements around contiki in preparation for RIOT support [\#302](https://github.com/FlowM2M/AwaLWM2M/pull/302) ([datachi7d](https://github.com/datachi7d))
- Change for older versions of GnuTLS that do not support AES-128-CCM-8 [\#290](https://github.com/FlowM2M/AwaLWM2M/pull/290) ([delmet](https://github.com/delmet))

**Fixed bugs:**

- Possible memory corruption in lwm2m\_bootstrap.c [\#298](https://github.com/FlowM2M/AwaLWM2M/issues/298)
- Bootstrapping and notification issues with awa-0.2.3 on Clickers to Ci40 [\#286](https://github.com/FlowM2M/AwaLWM2M/issues/286)
- Handle sendto\(\) function failure conditions [\#333](https://github.com/FlowM2M/AwaLWM2M/pull/333) ([pranit-sirsat-imgtec](https://github.com/pranit-sirsat-imgtec))
- Fix awa client explore [\#308](https://github.com/FlowM2M/AwaLWM2M/pull/308) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))

**Closed issues:**

- Cannot bootstrap more than 5 clickers [\#336](https://github.com/FlowM2M/AwaLWM2M/issues/336)
- Daemonizing Awa client fails to connect with the device server [\#321](https://github.com/FlowM2M/AwaLWM2M/issues/321)
- Path\_MakePath should handle ResourceInstanceID [\#317](https://github.com/FlowM2M/AwaLWM2M/issues/317)
- Bootstrap server security flag changes between the initial client write and server response [\#299](https://github.com/FlowM2M/AwaLWM2M/issues/299)
- TinyDTLS library as PSK encrypt layer doesn't work with 32 byte keys. [\#297](https://github.com/FlowM2M/AwaLWM2M/issues/297)
- Resolve of DNS is not working correctly on contiki. [\#296](https://github.com/FlowM2M/AwaLWM2M/issues/296)
- AwaClientSession\_Connect fails with AwaError\_IPCError when STDIN has been closed [\#288](https://github.com/FlowM2M/AwaLWM2M/issues/288)

**Merged pull requests:**

- awa\_clientd and static API bind to random port [\#338](https://github.com/FlowM2M/AwaLWM2M/pull/338) ([boyvinall](https://github.com/boyvinall))
- Contiki tinydtls fixes [\#337](https://github.com/FlowM2M/AwaLWM2M/pull/337) ([cheekyhalf](https://github.com/cheekyhalf))
- add CMAKE\_OPTIONS as docker build arg [\#328](https://github.com/FlowM2M/AwaLWM2M/pull/328) ([boyvinall](https://github.com/boyvinall))
- apply some markdownlint rules [\#327](https://github.com/FlowM2M/AwaLWM2M/pull/327) ([boyvinall](https://github.com/boyvinall))
- Update travis.yml to use pip's requirements.txt to install Python dependencies [\#326](https://github.com/FlowM2M/AwaLWM2M/pull/326) ([DavidAntliff](https://github.com/DavidAntliff))
- Python API: Make the API Python 2 & 3 compatible [\#324](https://github.com/FlowM2M/AwaLWM2M/pull/324) ([mtusnio](https://github.com/mtusnio))
- tinydtls: fix crash caused by null psk identity [\#322](https://github.com/FlowM2M/AwaLWM2M/pull/322) ([mkmk88](https://github.com/mkmk88))
- Patch tinydtls to accept 32 byte Psk  [\#320](https://github.com/FlowM2M/AwaLWM2M/pull/320) ([cheekyhalf](https://github.com/cheekyhalf))
- gitignore: Ignore generated .cmake [\#318](https://github.com/FlowM2M/AwaLWM2M/pull/318) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- Cleanup [\#316](https://github.com/FlowM2M/AwaLWM2M/pull/316) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- Add mbedTLS support [\#315](https://github.com/FlowM2M/AwaLWM2M/pull/315) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- awa-client-explore: Refactor code [\#312](https://github.com/FlowM2M/AwaLWM2M/pull/312) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- Add François Berder as a contributor [\#310](https://github.com/FlowM2M/AwaLWM2M/pull/310) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- Remove trailing whitespaces [\#309](https://github.com/FlowM2M/AwaLWM2M/pull/309) ([francois-berder-imgtec](https://github.com/francois-berder-imgtec))
- Rework the Contiki network address cache [\#307](https://github.com/FlowM2M/AwaLWM2M/pull/307) ([mtusnio](https://github.com/mtusnio))
- Fix Python API's observe request [\#306](https://github.com/FlowM2M/AwaLWM2M/pull/306) ([mtusnio](https://github.com/mtusnio))
- Fix for a potential buffer overflow during the bootstrap process [\#303](https://github.com/FlowM2M/AwaLWM2M/pull/303) ([mtusnio](https://github.com/mtusnio))
- Fix DNS lookup issue [\#300](https://github.com/FlowM2M/AwaLWM2M/pull/300) ([cheekyhalf](https://github.com/cheekyhalf))
- fix memory leak [\#295](https://github.com/FlowM2M/AwaLWM2M/pull/295) ([cheekyhalf](https://github.com/cheekyhalf))
- fix compiler warning and add option to control systemd service file installation [\#291](https://github.com/FlowM2M/AwaLWM2M/pull/291) ([abhimanyuv1](https://github.com/abhimanyuv1))
- Correct daemonize flag spelling [\#289](https://github.com/FlowM2M/AwaLWM2M/pull/289) ([Shpinkso](https://github.com/Shpinkso))
- Provide basic documentation [\#249](https://github.com/FlowM2M/AwaLWM2M/pull/249) ([seank-img](https://github.com/seank-img))

## [0.2.4](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.4) (2016-08-24)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.2.3...0.2.4)

**Implemented enhancements:**

- Change PUT/Replace on objects so it does not do delete/insert. [\#285](https://github.com/FlowM2M/AwaLWM2M/pull/285) ([delmet](https://github.com/delmet))
- Implement AwaServerWriteOperation\_AddArrayValueAsInteger [\#281](https://github.com/FlowM2M/AwaLWM2M/pull/281) ([delmet](https://github.com/delmet))
- Make contiki version of function NetworkAddress\_New consistent with linux one [\#280](https://github.com/FlowM2M/AwaLWM2M/pull/280) ([delmet](https://github.com/delmet))
- Move daemon code out of the core [\#275](https://github.com/FlowM2M/AwaLWM2M/pull/275) ([datachi7d](https://github.com/datachi7d))
- Under GnuTLS allow DTLS server to accept clients with either PSK or Certs  [\#268](https://github.com/FlowM2M/AwaLWM2M/pull/268) ([delmet](https://github.com/delmet))
- Get registration status for static client API [\#266](https://github.com/FlowM2M/AwaLWM2M/pull/266) ([rlatchem](https://github.com/rlatchem))

**Fixed bugs:**

- bootstrap configuration file load fail [\#276](https://github.com/FlowM2M/AwaLWM2M/issues/276)
- Searching for a position in the Security Object /0 [\#269](https://github.com/FlowM2M/AwaLWM2M/issues/269)
- Ignore blank lines in bootstrap config files [\#282](https://github.com/FlowM2M/AwaLWM2M/pull/282) ([delmet](https://github.com/delmet))
- Fix coverity issue 140813 [\#271](https://github.com/FlowM2M/AwaLWM2M/pull/271) ([delmet](https://github.com/delmet))
- Change comparison on addresses to also check ports \(See issue \#269\) [\#270](https://github.com/FlowM2M/AwaLWM2M/pull/270) ([delmet](https://github.com/delmet))

**Closed issues:**

- error messages while call to awa-server-explore [\#284](https://github.com/FlowM2M/AwaLWM2M/issues/284)
- Missing mail list or contact information [\#278](https://github.com/FlowM2M/AwaLWM2M/issues/278)

**Merged pull requests:**

- Update static API docs for erbium in contiki [\#283](https://github.com/FlowM2M/AwaLWM2M/pull/283) ([datachi7d](https://github.com/datachi7d))
- Change replace \(PUT\) support for resources only, not to do delete/insert [\#273](https://github.com/FlowM2M/AwaLWM2M/pull/273) ([delmet](https://github.com/delmet))
- Add NZOSA news. Add origin statement. [\#279](https://github.com/FlowM2M/AwaLWM2M/pull/279) ([DavidAntliff](https://github.com/DavidAntliff))
- Update CONTRIBUTORS. [\#277](https://github.com/FlowM2M/AwaLWM2M/pull/277) ([DavidAntliff](https://github.com/DavidAntliff))
- Bump version [\#267](https://github.com/FlowM2M/AwaLWM2M/pull/267) ([delmet](https://github.com/delmet))
- Branding image replaced [\#265](https://github.com/FlowM2M/AwaLWM2M/pull/265) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))

## [0.2.3](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.3) (2016-07-28)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.2.2...0.2.3)

**Implemented enhancements:**

- Wire up support for Certificate file for client daemon [\#263](https://github.com/FlowM2M/AwaLWM2M/pull/263) ([delmet](https://github.com/delmet))
- Actually make use of new default Content Type command line arg [\#262](https://github.com/FlowM2M/AwaLWM2M/pull/262) ([delmet](https://github.com/delmet))
- Change for TinyDTLS abstraction for Contiki [\#261](https://github.com/FlowM2M/AwaLWM2M/pull/261) ([delmet](https://github.com/delmet))
- Add option to turning off building examples [\#260](https://github.com/FlowM2M/AwaLWM2M/pull/260) ([delmet](https://github.com/delmet))
- Add compiler error if GnuTLS does not support DTLS [\#259](https://github.com/FlowM2M/AwaLWM2M/pull/259) ([delmet](https://github.com/delmet))
- Allow setting default content type for client daemon and static client [\#258](https://github.com/FlowM2M/AwaLWM2M/pull/258) ([delmet](https://github.com/delmet))

**Fixed bugs:**

- Seed random generator in daemons [\#264](https://github.com/FlowM2M/AwaLWM2M/pull/264) ([delmet](https://github.com/delmet))

**Closed issues:**

- Content Type and the data serialization [\#257](https://github.com/FlowM2M/AwaLWM2M/issues/257)
- doc directory is becoming a little untidy due to number of different files in a flat structure [\#245](https://github.com/FlowM2M/AwaLWM2M/issues/245)

**Merged pull requests:**

- Fix coverity issues 135790, 13579 and 135805 [\#256](https://github.com/FlowM2M/AwaLWM2M/pull/256) ([delmet](https://github.com/delmet))
- Version bump [\#255](https://github.com/FlowM2M/AwaLWM2M/pull/255) ([delmet](https://github.com/delmet))

## [0.2.2](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.2) (2016-07-22)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.2.1...0.2.2)

**Implemented enhancements:**

- Fix contiki makefile for removed ojects [\#254](https://github.com/FlowM2M/AwaLWM2M/pull/254) ([delmet](https://github.com/delmet))
- Change contiki to use network abstraction and awa erbium [\#253](https://github.com/FlowM2M/AwaLWM2M/pull/253) ([delmet](https://github.com/delmet))
- Remove device, firmware, location and connectivity objects from the client daemon [\#247](https://github.com/FlowM2M/AwaLWM2M/pull/247) ([seank-img](https://github.com/seank-img))
- Add support to client daemon to pass in PSK arguments via command line [\#240](https://github.com/FlowM2M/AwaLWM2M/pull/240) ([delmet](https://github.com/delmet))

**Fixed bugs:**

- Awa static client fails to retry after registration failed timeout [\#149](https://github.com/FlowM2M/AwaLWM2M/issues/149)

**Merged pull requests:**

- Modified README.md and CONTRIBUTING.md [\#251](https://github.com/FlowM2M/AwaLWM2M/pull/251) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Disable JSON support by default [\#250](https://github.com/FlowM2M/AwaLWM2M/pull/250) ([seank-img](https://github.com/seank-img))
- Move doc images to their own subdirectory. Closes issue \#245. [\#246](https://github.com/FlowM2M/AwaLWM2M/pull/246) ([DavidAntliff](https://github.com/DavidAntliff))
- Update CONTRIBUTORS [\#242](https://github.com/FlowM2M/AwaLWM2M/pull/242) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix reference to IPC documentation [\#241](https://github.com/FlowM2M/AwaLWM2M/pull/241) ([cdewbery](https://github.com/cdewbery))
- Version bump [\#239](https://github.com/FlowM2M/AwaLWM2M/pull/239) ([delmet](https://github.com/delmet))

## [0.2.1](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.1) (2016-07-11)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.2.0...0.2.1)

**Implemented enhancements:**

- Add support to reset DTLS sessions [\#238](https://github.com/FlowM2M/AwaLWM2M/pull/238) ([delmet](https://github.com/delmet))
- Change log output from CyaSSL [\#234](https://github.com/FlowM2M/AwaLWM2M/pull/234) ([rlatchem](https://github.com/rlatchem))

**Fixed bugs:**

- Fix contiki client build [\#236](https://github.com/FlowM2M/AwaLWM2M/pull/236) ([seank-img](https://github.com/seank-img))
- Fix GNUTLS CertificateFormat definition [\#235](https://github.com/FlowM2M/AwaLWM2M/pull/235) ([delmet](https://github.com/delmet))

**Merged pull requests:**

- Version bump [\#237](https://github.com/FlowM2M/AwaLWM2M/pull/237) ([seank-img](https://github.com/seank-img))

## [0.2.0](https://github.com/FlowM2M/AwaLWM2M/tree/0.2.0) (2016-07-08)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.10...0.2.0)

**Implemented enhancements:**

- bootstrap-\*.config files will be renamed to \*.bsc [\#219](https://github.com/FlowM2M/AwaLWM2M/issues/219)
- Add script for updating docs repository. [\#229](https://github.com/FlowM2M/AwaLWM2M/pull/229) ([DavidAntliff](https://github.com/DavidAntliff))
- Add to Static API ability to set default Certificate or PSK [\#228](https://github.com/FlowM2M/AwaLWM2M/pull/228) ([delmet](https://github.com/delmet))
- Set default CoAP library as Erbium. [\#227](https://github.com/FlowM2M/AwaLWM2M/pull/227) ([DavidAntliff](https://github.com/DavidAntliff))
- Add additional DTLS support [\#226](https://github.com/FlowM2M/AwaLWM2M/pull/226) ([DavidAntliff](https://github.com/DavidAntliff))
- Update Cyassl for Wifire [\#225](https://github.com/FlowM2M/AwaLWM2M/pull/225) ([rlatchem](https://github.com/rlatchem))
- Server objdef loading [\#222](https://github.com/FlowM2M/AwaLWM2M/pull/222) ([DavidAntliff](https://github.com/DavidAntliff))
- Support multiple instances of --objDefs/-o option with awa\_clientd [\#221](https://github.com/FlowM2M/AwaLWM2M/pull/221) ([DavidAntliff](https://github.com/DavidAntliff))
- Gengetopts daemons [\#220](https://github.com/FlowM2M/AwaLWM2M/pull/220) ([DavidAntliff](https://github.com/DavidAntliff))
- CyaSSL/WolfSSL working for client DTLS with Certificate [\#218](https://github.com/FlowM2M/AwaLWM2M/pull/218) ([delmet](https://github.com/delmet))
- Support client objdef loading from xml file [\#217](https://github.com/FlowM2M/AwaLWM2M/pull/217) ([DavidAntliff](https://github.com/DavidAntliff))
- Add Tinydtls support [\#213](https://github.com/FlowM2M/AwaLWM2M/pull/213) ([delmet](https://github.com/delmet))
- Explore tool XML export [\#211](https://github.com/FlowM2M/AwaLWM2M/pull/211) ([datachi7d](https://github.com/datachi7d))
- Add support for pic32 platform [\#202](https://github.com/FlowM2M/AwaLWM2M/pull/202) ([rlatchem](https://github.com/rlatchem))
- Fix tests for Erbium [\#198](https://github.com/FlowM2M/AwaLWM2M/pull/198) ([rlatchem](https://github.com/rlatchem))
- Erbium and Static API test improvements [\#197](https://github.com/FlowM2M/AwaLWM2M/pull/197) ([seank-img](https://github.com/seank-img))
- Add docker override variables to permit customisation of build [\#196](https://github.com/FlowM2M/AwaLWM2M/pull/196) ([DavidAntliff](https://github.com/DavidAntliff))
- Add reporting of CoAP and DTLS libraries to daemons. [\#194](https://github.com/FlowM2M/AwaLWM2M/pull/194) ([DavidAntliff](https://github.com/DavidAntliff))
- Add Erbium server support [\#193](https://github.com/FlowM2M/AwaLWM2M/pull/193) ([rlatchem](https://github.com/rlatchem))
- Add initial Erbium support to Awa [\#190](https://github.com/FlowM2M/AwaLWM2M/pull/190) ([rlatchem](https://github.com/rlatchem))

**Fixed bugs:**

- Awa server and client explore tools show "ResourceOperationToString table is wrong size!" Error [\#208](https://github.com/FlowM2M/AwaLWM2M/issues/208)
- For erbium make sure coap message id is random [\#232](https://github.com/FlowM2M/AwaLWM2M/pull/232) ([delmet](https://github.com/delmet))
- Fix init code for older gcc compiler [\#231](https://github.com/FlowM2M/AwaLWM2M/pull/231) ([rlatchem](https://github.com/rlatchem))
- Fix Coverity issue 130249. [\#230](https://github.com/FlowM2M/AwaLWM2M/pull/230) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix coverity issues 125944 and 127105. [\#215](https://github.com/FlowM2M/AwaLWM2M/pull/215) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix server and client explore tool [\#210](https://github.com/FlowM2M/AwaLWM2M/pull/210) ([seank-img](https://github.com/seank-img))
- Fix valgrind errors in bootstrap daemon. [\#206](https://github.com/FlowM2M/AwaLWM2M/pull/206) ([DavidAntliff](https://github.com/DavidAntliff))
- Prevent duplicate retry on bootstrap failed [\#203](https://github.com/FlowM2M/AwaLWM2M/pull/203) ([rlatchem](https://github.com/rlatchem))
- Fix tests for Erbium [\#198](https://github.com/FlowM2M/AwaLWM2M/pull/198) ([rlatchem](https://github.com/rlatchem))
- Erbium and Static API test improvements [\#197](https://github.com/FlowM2M/AwaLWM2M/pull/197) ([seank-img](https://github.com/seank-img))

**Closed issues:**

- Define custom objects when awa-serverd starts [\#200](https://github.com/FlowM2M/AwaLWM2M/issues/200)

**Merged pull requests:**

- Bootstrap multiple config docs [\#216](https://github.com/FlowM2M/AwaLWM2M/pull/216) ([DavidAntliff](https://github.com/DavidAntliff))
- Updated static api examples [\#209](https://github.com/FlowM2M/AwaLWM2M/pull/209) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Update README.md [\#191](https://github.com/FlowM2M/AwaLWM2M/pull/191) ([thetoster](https://github.com/thetoster))

## [0.1.10](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.10) (2016-06-19)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.9...0.1.10)

**Implemented enhancements:**

- Speed up unit tests [\#165](https://github.com/FlowM2M/AwaLWM2M/issues/165)
- Docker coverage [\#189](https://github.com/FlowM2M/AwaLWM2M/pull/189) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix code coverage report generation for docker build. [\#187](https://github.com/FlowM2M/AwaLWM2M/pull/187) ([DavidAntliff](https://github.com/DavidAntliff))
- Create Dockerfile for CI jobs. [\#185](https://github.com/FlowM2M/AwaLWM2M/pull/185) ([DavidAntliff](https://github.com/DavidAntliff))
- Add support for systemd [\#181](https://github.com/FlowM2M/AwaLWM2M/pull/181) ([DavidAntliff](https://github.com/DavidAntliff))
- Add API functions to set a session's default Connect/Disconnect timeout [\#173](https://github.com/FlowM2M/AwaLWM2M/pull/173) ([DavidAntliff](https://github.com/DavidAntliff))
- Additional test optimisations for \#165. [\#172](https://github.com/FlowM2M/AwaLWM2M/pull/172) ([DavidAntliff](https://github.com/DavidAntliff))
- Add Dockerfile creation. [\#171](https://github.com/FlowM2M/AwaLWM2M/pull/171) ([DavidAntliff](https://github.com/DavidAntliff))
- Speed up tests by removing conservative waits. Refer \#165. [\#169](https://github.com/FlowM2M/AwaLWM2M/pull/169) ([DavidAntliff](https://github.com/DavidAntliff))
- Improve test logging, to help correlate IPC Session IDs with test names [\#150](https://github.com/FlowM2M/AwaLWM2M/pull/150) ([DavidAntliff](https://github.com/DavidAntliff))
- Avoid deprecated function warning by using RAII-wrapped mkstemp inste… [\#148](https://github.com/FlowM2M/AwaLWM2M/pull/148) ([DavidAntliff](https://github.com/DavidAntliff))

**Fixed bugs:**

- Update Connect/Disconnect documentation. [\#175](https://github.com/FlowM2M/AwaLWM2M/issues/175)
- Fix cppcheck-1.72 issues. [\#188](https://github.com/FlowM2M/AwaLWM2M/pull/188) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix Coverity issues 115871 and 115872 \(duplicates \#174\) [\#179](https://github.com/FlowM2M/AwaLWM2M/pull/179) ([seank-img](https://github.com/seank-img))
- Minor fixes [\#176](https://github.com/FlowM2M/AwaLWM2M/pull/176) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix Coverity issues 115871 and 115872. [\#174](https://github.com/FlowM2M/AwaLWM2M/pull/174) ([DavidAntliff](https://github.com/DavidAntliff))
- Filter deprecation doxygen warnings to prevent build breaking with Ubuntu 15.10+. [\#168](https://github.com/FlowM2M/AwaLWM2M/pull/168) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix ClientUpdateEvent test. [\#151](https://github.com/FlowM2M/AwaLWM2M/pull/151) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix illegal memory access [\#147](https://github.com/FlowM2M/AwaLWM2M/pull/147) ([cdewbery](https://github.com/cdewbery))
- Added guards around Lwm2mCore\_DebugPrintSockAddr which is not supported by CONTIKI [\#146](https://github.com/FlowM2M/AwaLWM2M/pull/146) ([rolandbewick](https://github.com/rolandbewick))

**Closed issues:**

- Improve reference to examples [\#161](https://github.com/FlowM2M/AwaLWM2M/issues/161)
- Add Contiki documentation [\#159](https://github.com/FlowM2M/AwaLWM2M/issues/159)
- Elaborate on importance of bootstrap config files [\#157](https://github.com/FlowM2M/AwaLWM2M/issues/157)

**Merged pull requests:**

- Filter out coverage of \_cmdline.c files. [\#192](https://github.com/FlowM2M/AwaLWM2M/pull/192) ([DavidAntliff](https://github.com/DavidAntliff))
- Update docker.md for styling and branding [\#186](https://github.com/FlowM2M/AwaLWM2M/pull/186) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Add documentation for Docker. [\#184](https://github.com/FlowM2M/AwaLWM2M/pull/184) ([DavidAntliff](https://github.com/DavidAntliff))
- Version bump [\#183](https://github.com/FlowM2M/AwaLWM2M/pull/183) ([seank-img](https://github.com/seank-img))
- Documentation updates [\#182](https://github.com/FlowM2M/AwaLWM2M/pull/182) ([seank-img](https://github.com/seank-img))
- Fix Coverity issue 114167. [\#153](https://github.com/FlowM2M/AwaLWM2M/pull/153) ([DavidAntliff](https://github.com/DavidAntliff))
- Documentation review updates [\#152](https://github.com/FlowM2M/AwaLWM2M/pull/152) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))

## [0.1.9](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.9) (2016-05-23)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.8...0.1.9)

**Implemented enhancements:**

- Small fixes to address family in Awa Daemons [\#135](https://github.com/FlowM2M/AwaLWM2M/pull/135) ([rolandbewick](https://github.com/rolandbewick))
- Implement Server Events. [\#137](https://github.com/FlowM2M/AwaLWM2M/pull/137) ([DavidAntliff](https://github.com/DavidAntliff))
- cmake installation changes [\#134](https://github.com/FlowM2M/AwaLWM2M/pull/134) ([DavidAntliff](https://github.com/DavidAntliff))

**Fixed bugs:**

- Small fixes to address family in Awa Daemons [\#135](https://github.com/FlowM2M/AwaLWM2M/pull/135) ([rolandbewick](https://github.com/rolandbewick))
- Time values should be signed, and a signed 32-bit long on MIPS is too… [\#144](https://github.com/FlowM2M/AwaLWM2M/pull/144) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix set resource storage with pointer handling for opaque types [\#139](https://github.com/FlowM2M/AwaLWM2M/pull/139) ([seank-img](https://github.com/seank-img))
- cmake installation changes [\#134](https://github.com/FlowM2M/AwaLWM2M/pull/134) ([DavidAntliff](https://github.com/DavidAntliff))

**Merged pull requests:**

- Bumped version to 0.1.9. [\#145](https://github.com/FlowM2M/AwaLWM2M/pull/145) ([DavidAntliff](https://github.com/DavidAntliff))
- Update default install paths in tutorial examples [\#143](https://github.com/FlowM2M/AwaLWM2M/pull/143) ([rlatchem](https://github.com/rlatchem))
- Added Tony and Rory to CONTRIBUTORS [\#142](https://github.com/FlowM2M/AwaLWM2M/pull/142) ([rolandbewick](https://github.com/rolandbewick))
- Updated Awa static API md image [\#141](https://github.com/FlowM2M/AwaLWM2M/pull/141) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Update Awa\_Static\_API.md [\#138](https://github.com/FlowM2M/AwaLWM2M/pull/138) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))

## [0.1.8](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.8) (2016-05-04)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.7...0.1.8)

**Implemented enhancements:**

- Remove deprecated functions prior to 0.1.8 release. [\#131](https://github.com/FlowM2M/AwaLWM2M/pull/131) ([DavidAntliff](https://github.com/DavidAntliff))
- Static API changes [\#124](https://github.com/FlowM2M/AwaLWM2M/pull/124) ([DavidAntliff](https://github.com/DavidAntliff))
- Add Static API delete object and resource [\#123](https://github.com/FlowM2M/AwaLWM2M/pull/123) ([seank-img](https://github.com/seank-img))
- Fixed xmlif\_AddDefaultsForMissingMandatoryValues not returning -1 on error. [\#118](https://github.com/FlowM2M/AwaLWM2M/pull/118) ([rolandbewick](https://github.com/rolandbewick))

**Fixed bugs:**

- Fix buffer overrun detected by coverity [\#116](https://github.com/FlowM2M/AwaLWM2M/pull/116) ([cdewbery](https://github.com/cdewbery))
- Fix Static API define behaviour before init and add test case [\#129](https://github.com/FlowM2M/AwaLWM2M/pull/129) ([seank-img](https://github.com/seank-img))
- Fix valgrind error when CoAP transaction times out or response path d… [\#126](https://github.com/FlowM2M/AwaLWM2M/pull/126) ([seank-img](https://github.com/seank-img))
- Fix use of unittest's doCleanups\(\) mechanism. [\#122](https://github.com/FlowM2M/AwaLWM2M/pull/122) ([DavidAntliff](https://github.com/DavidAntliff))
- Return AwaLWM2MError\_Timeout via IPC when a CoAP request timesout [\#119](https://github.com/FlowM2M/AwaLWM2M/pull/119) ([cdewbery](https://github.com/cdewbery))
- Fixed xmlif\\_AddDefaultsForMissingMandatoryValues not returning -1 on error. [\#118](https://github.com/FlowM2M/AwaLWM2M/pull/118) ([rolandbewick](https://github.com/rolandbewick))

**Merged pull requests:**

- Updated README.md [\#117](https://github.com/FlowM2M/AwaLWM2M/pull/117) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Fixing 3rd party guide link [\#114](https://github.com/FlowM2M/AwaLWM2M/pull/114) ([andreibosco](https://github.com/andreibosco))
- Documentation update. [\#132](https://github.com/FlowM2M/AwaLWM2M/pull/132) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix define objects in static examples [\#128](https://github.com/FlowM2M/AwaLWM2M/pull/128) ([rlatchem](https://github.com/rlatchem))
- Version bump [\#127](https://github.com/FlowM2M/AwaLWM2M/pull/127) ([seank-img](https://github.com/seank-img))
- Add static API guide and LWM2M overview [\#125](https://github.com/FlowM2M/AwaLWM2M/pull/125) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Removed creator logo from doxygen output [\#121](https://github.com/FlowM2M/AwaLWM2M/pull/121) ([cdewbery](https://github.com/cdewbery))
- Minor documentation update. [\#120](https://github.com/FlowM2M/AwaLWM2M/pull/120) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix some coverity issues raised in April 11th scan. [\#115](https://github.com/FlowM2M/AwaLWM2M/pull/115) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix some coverity issues raised in April 10th scan. [\#113](https://github.com/FlowM2M/AwaLWM2M/pull/113) ([DavidAntliff](https://github.com/DavidAntliff))

## [0.1.7](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.7) (2016-04-11)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.6...0.1.7)

**Implemented enhancements:**

- Add Coverity Scan badge. [\#112](https://github.com/FlowM2M/AwaLWM2M/pull/112) ([DavidAntliff](https://github.com/DavidAntliff))
- Fixes and improvements to Static API [\#110](https://github.com/FlowM2M/AwaLWM2M/pull/110) ([seank-img](https://github.com/seank-img))

**Fixed bugs:**

- Fixes and improvements to Static API [\#110](https://github.com/FlowM2M/AwaLWM2M/pull/110) ([seank-img](https://github.com/seank-img))

**Merged pull requests:**

- Version bump [\#111](https://github.com/FlowM2M/AwaLWM2M/pull/111) ([seank-img](https://github.com/seank-img))
- Update starters\_guide.md - typo fixed [\#109](https://github.com/FlowM2M/AwaLWM2M/pull/109) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Update starters\_guide.md [\#107](https://github.com/FlowM2M/AwaLWM2M/pull/107) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Valgrind PR CI testing. [\#99](https://github.com/FlowM2M/AwaLWM2M/pull/99) ([DavidAntliff](https://github.com/DavidAntliff))

## [0.1.6](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.6) (2016-04-05)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.5...0.1.6)

**Implemented enhancements:**

- Refactor Core handlers and add error messages for handlers [\#104](https://github.com/FlowM2M/AwaLWM2M/pull/104) ([seank-img](https://github.com/seank-img))
- Additional Coverity fixes [\#103](https://github.com/FlowM2M/AwaLWM2M/pull/103) ([DavidAntliff](https://github.com/DavidAntliff))
- Disable tmpnam deprecation warning, as current design requires a uniq… [\#102](https://github.com/FlowM2M/AwaLWM2M/pull/102) ([DavidAntliff](https://github.com/DavidAntliff))
- Add --version/-V option to daemons to report version number and then … [\#100](https://github.com/FlowM2M/AwaLWM2M/pull/100) ([DavidAntliff](https://github.com/DavidAntliff))
- Fixed defaults not being added correctly to CoAP create packet from server XML handler [\#98](https://github.com/FlowM2M/AwaLWM2M/pull/98) ([rolandbewick](https://github.com/rolandbewick))
- Added parameterised set/write operation test validation and fixed set not doing a replace on array resources. [\#96](https://github.com/FlowM2M/AwaLWM2M/pull/96) ([rolandbewick](https://github.com/rolandbewick))

**Fixed bugs:**

- Fix bug in server where creating an object will populate optional resources. [\#105](https://github.com/FlowM2M/AwaLWM2M/pull/105) ([seank-img](https://github.com/seank-img))
- Additional Coverity fixes [\#103](https://github.com/FlowM2M/AwaLWM2M/pull/103) ([DavidAntliff](https://github.com/DavidAntliff))
- Fixed AwaServerObservation\_New not allocating memory for ClientID. [\#101](https://github.com/FlowM2M/AwaLWM2M/pull/101) ([rolandbewick](https://github.com/rolandbewick))
- Fixed defaults not being added correctly to CoAP create packet from server XML handler [\#98](https://github.com/FlowM2M/AwaLWM2M/pull/98) ([rolandbewick](https://github.com/rolandbewick))
- Added parameterised set/write operation test validation and fixed set not doing a replace on array resources. [\#96](https://github.com/FlowM2M/AwaLWM2M/pull/96) ([rolandbewick](https://github.com/rolandbewick))
- Rename AwaStaticClient\_RegisterObjectWithHandler to AwaStaticClient\_D… [\#95](https://github.com/FlowM2M/AwaLWM2M/pull/95) ([cdewbery](https://github.com/cdewbery))

**Merged pull requests:**

- Bump version [\#106](https://github.com/FlowM2M/AwaLWM2M/pull/106) ([seank-img](https://github.com/seank-img))

## [0.1.5](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.5) (2016-03-23)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.4...0.1.5)

**Implemented enhancements:**

- Update doxygen title. [\#92](https://github.com/FlowM2M/AwaLWM2M/pull/92) ([DavidAntliff](https://github.com/DavidAntliff))
- Add license to python source code and scripts [\#90](https://github.com/FlowM2M/AwaLWM2M/pull/90) ([seank-img](https://github.com/seank-img))
- Improve doxygen documentation. [\#89](https://github.com/FlowM2M/AwaLWM2M/pull/89) ([DavidAntliff](https://github.com/DavidAntliff))
- Static API negative test cases and improve Static API error handling [\#87](https://github.com/FlowM2M/AwaLWM2M/pull/87) ([seank-img](https://github.com/seank-img))
- Deleted old server create tool and updated server delete tool [\#86](https://github.com/FlowM2M/AwaLWM2M/pull/86) ([rolandbewick](https://github.com/rolandbewick))
- Rework core and API to use non-terminated strings [\#84](https://github.com/FlowM2M/AwaLWM2M/pull/84) ([seank-img](https://github.com/seank-img))
- Add support for AwaStaticClient\_RegisterResourceWithPointerArray\(\) [\#82](https://github.com/FlowM2M/AwaLWM2M/pull/82) ([cdewbery](https://github.com/cdewbery))
- Rework factory bootstrap to take next available object instance ID [\#81](https://github.com/FlowM2M/AwaLWM2M/pull/81) ([seank-img](https://github.com/seank-img))
- Static API review and improvements [\#80](https://github.com/FlowM2M/AwaLWM2M/pull/80) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix valgrind errors due to missing serialisers for parameterised tests. [\#78](https://github.com/FlowM2M/AwaLWM2M/pull/78) ([DavidAntliff](https://github.com/DavidAntliff))
- Removed duplicated enum from the core and first pass of static API documentation. [\#77](https://github.com/FlowM2M/AwaLWM2M/pull/77) ([rolandbewick](https://github.com/rolandbewick))
- Fix spelling of "CoAP". [\#75](https://github.com/FlowM2M/AwaLWM2M/pull/75) ([DavidAntliff](https://github.com/DavidAntliff))
- Improve error handling of invalid within static API [\#71](https://github.com/FlowM2M/AwaLWM2M/pull/71) ([cdewbery](https://github.com/cdewbery))
- Added AwaStaticClient\_ResourceChanged [\#69](https://github.com/FlowM2M/AwaLWM2M/pull/69) ([rolandbewick](https://github.com/rolandbewick))
- Remove AwaStaticResourceType, AwaResourceType is now used everywhere [\#67](https://github.com/FlowM2M/AwaLWM2M/pull/67) ([cdewbery](https://github.com/cdewbery))
- Small fixes/tidy ups to static API example and added SetLogLevel function [\#66](https://github.com/FlowM2M/AwaLWM2M/pull/66) ([rolandbewick](https://github.com/rolandbewick))
- Add example contiki application to documentation [\#64](https://github.com/FlowM2M/AwaLWM2M/pull/64) ([cdewbery](https://github.com/cdewbery))
- Build libawa\_static.so, Add static-client-tutorial to documentation [\#63](https://github.com/FlowM2M/AwaLWM2M/pull/63) ([cdewbery](https://github.com/cdewbery))

**Fixed bugs:**

- Fix valgrind error in Static API test [\#94](https://github.com/FlowM2M/AwaLWM2M/pull/94) ([seank-img](https://github.com/seank-img))
- Fix json and plaintext serialises not NULL terminating strings correctly [\#85](https://github.com/FlowM2M/AwaLWM2M/pull/85) ([cdewbery](https://github.com/cdewbery))
- Fix valgrind errors due to missing serialisers for parameterised tests. [\#78](https://github.com/FlowM2M/AwaLWM2M/pull/78) ([DavidAntliff](https://github.com/DavidAntliff))
- Add missing valgrind tests. [\#76](https://github.com/FlowM2M/AwaLWM2M/pull/76) ([DavidAntliff](https://github.com/DavidAntliff))
- Improve error handling of invalid within static API [\#71](https://github.com/FlowM2M/AwaLWM2M/pull/71) ([cdewbery](https://github.com/cdewbery))
- Small fixes/tidy ups to static API example and added SetLogLevel function [\#66](https://github.com/FlowM2M/AwaLWM2M/pull/66) ([rolandbewick](https://github.com/rolandbewick))

**Merged pull requests:**

- Add license to tutorials [\#91](https://github.com/FlowM2M/AwaLWM2M/pull/91) ([seank-img](https://github.com/seank-img))
- Add license to source files. [\#88](https://github.com/FlowM2M/AwaLWM2M/pull/88) ([seank-img](https://github.com/seank-img))
- Enable gcov for test\_core\_runner [\#83](https://github.com/FlowM2M/AwaLWM2M/pull/83) ([cdewbery](https://github.com/cdewbery))
- Add docs to CI build. [\#79](https://github.com/FlowM2M/AwaLWM2M/pull/79) ([DavidAntliff](https://github.com/DavidAntliff))
- Added static client example using handlers. [\#74](https://github.com/FlowM2M/AwaLWM2M/pull/74) ([rolandbewick](https://github.com/rolandbewick))
- Updated coding\_style.md - minor edit [\#73](https://github.com/FlowM2M/AwaLWM2M/pull/73) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Add API tutorials as per documentation to api/example/tutorials [\#72](https://github.com/FlowM2M/AwaLWM2M/pull/72) ([cdewbery](https://github.com/cdewbery))
- Add build/license badges to README [\#70](https://github.com/FlowM2M/AwaLWM2M/pull/70) ([cdewbery](https://github.com/cdewbery))
- Add AWA\_OPAQUE macro to Awa static API [\#68](https://github.com/FlowM2M/AwaLWM2M/pull/68) ([cdewbery](https://github.com/cdewbery))
- Add link to development task list on wiki to the README [\#65](https://github.com/FlowM2M/AwaLWM2M/pull/65) ([cdewbery](https://github.com/cdewbery))
- Update version [\#93](https://github.com/FlowM2M/AwaLWM2M/pull/93) ([seank-img](https://github.com/seank-img))

## [0.1.4](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.4) (2016-03-15)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.3...0.1.4)

**Implemented enhancements:**

- Provide build-time switch to disable JSON support. [\#61](https://github.com/FlowM2M/AwaLWM2M/pull/61) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix up include path for awa/static.h [\#57](https://github.com/FlowM2M/AwaLWM2M/pull/57) ([cdewbery](https://github.com/cdewbery))
- Untangle static api dependencies [\#56](https://github.com/FlowM2M/AwaLWM2M/pull/56) ([seank-img](https://github.com/seank-img))
- Add command for travis-ci to run tests. [\#55](https://github.com/FlowM2M/AwaLWM2M/pull/55) ([DavidAntliff](https://github.com/DavidAntliff))
- Add tests for all types of single instance resources in Static API handler [\#52](https://github.com/FlowM2M/AwaLWM2M/pull/52) ([seank-img](https://github.com/seank-img))
- Fixes for Static API tests [\#49](https://github.com/FlowM2M/AwaLWM2M/pull/49) ([seank-img](https://github.com/seank-img))
- Basis for parameterised tests of Static API [\#48](https://github.com/FlowM2M/AwaLWM2M/pull/48) ([seank-img](https://github.com/seank-img))
- Updated makefiles for static api so that it builds for contiki [\#46](https://github.com/FlowM2M/AwaLWM2M/pull/46) ([cdewbery](https://github.com/cdewbery))
- Add support for out-of-memory tests. [\#40](https://github.com/FlowM2M/AwaLWM2M/pull/40) ([DavidAntliff](https://github.com/DavidAntliff))
- Basis for Static API [\#39](https://github.com/FlowM2M/AwaLWM2M/pull/39) ([seank-img](https://github.com/seank-img))
- Add travis yml [\#37](https://github.com/FlowM2M/AwaLWM2M/pull/37) ([cdewbery](https://github.com/cdewbery))
- Add support for branch coverage reporting. [\#31](https://github.com/FlowM2M/AwaLWM2M/pull/31) ([DavidAntliff](https://github.com/DavidAntliff))
- Double IPC Session Connect timeout to 10 seconds. [\#30](https://github.com/FlowM2M/AwaLWM2M/pull/30) ([DavidAntliff](https://github.com/DavidAntliff))
- Add support for the Cobertura code-coverage Jenkins plugin. [\#27](https://github.com/FlowM2M/AwaLWM2M/pull/27) ([DavidAntliff](https://github.com/DavidAntliff))
- Refactor get length into read resource [\#26](https://github.com/FlowM2M/AwaLWM2M/pull/26) ([seank-img](https://github.com/seank-img))

**Fixed bugs:**

- Fix support for Python tools tests. [\#60](https://github.com/FlowM2M/AwaLWM2M/pull/60) ([DavidAntliff](https://github.com/DavidAntliff))
- Fixed PUT / POST handling on multiple instance and mandatory resources. [\#51](https://github.com/FlowM2M/AwaLWM2M/pull/51) ([rolandbewick](https://github.com/rolandbewick))
- Fix default pmin value as this was preventing notifications on the cl… [\#50](https://github.com/FlowM2M/AwaLWM2M/pull/50) ([cdewbery](https://github.com/cdewbery))
- Add missing header file so ssizet is defined when building on other p… [\#47](https://github.com/FlowM2M/AwaLWM2M/pull/47) ([cdewbery](https://github.com/cdewbery))
- Fix TLV buffer overflow [\#43](https://github.com/FlowM2M/AwaLWM2M/pull/43) ([cdewbery](https://github.com/cdewbery))
- Fix issues highlighted by coverity.com on commit 05879c4. [\#38](https://github.com/FlowM2M/AwaLWM2M/pull/38) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix debug message in GetRequest handler [\#36](https://github.com/FlowM2M/AwaLWM2M/pull/36) ([cdewbery](https://github.com/cdewbery))
- Fix error when attempting to create instance of object 7 [\#33](https://github.com/FlowM2M/AwaLWM2M/pull/33) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix missing filenames and line numbers with non-info log messages. [\#28](https://github.com/FlowM2M/AwaLWM2M/pull/28) ([DavidAntliff](https://github.com/DavidAntliff))

**Merged pull requests:**

- Untangle static api dependencies - do not merge [\#53](https://github.com/FlowM2M/AwaLWM2M/pull/53) ([cdewbery](https://github.com/cdewbery))
- Updated blurb on doxygen landing page [\#45](https://github.com/FlowM2M/AwaLWM2M/pull/45) ([cdewbery](https://github.com/cdewbery))
- Document navigation [\#44](https://github.com/FlowM2M/AwaLWM2M/pull/44) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Editing documentation to differentiate the Awa LightweightM2M product [\#41](https://github.com/FlowM2M/AwaLWM2M/pull/41) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Awa lightweight m2 m dev branch [\#32](https://github.com/FlowM2M/AwaLWM2M/pull/32) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Changed contributing - removed rebase info to another file and linked… [\#29](https://github.com/FlowM2M/AwaLWM2M/pull/29) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Bumped version to 0.1.4 in preparation for release. [\#62](https://github.com/FlowM2M/AwaLWM2M/pull/62) ([DavidAntliff](https://github.com/DavidAntliff))
- Revert out-of-memory tests and support. [\#54](https://github.com/FlowM2M/AwaLWM2M/pull/54) ([DavidAntliff](https://github.com/DavidAntliff))

## [0.1.3](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.3) (2016-03-03)
[Full Changelog](https://github.com/FlowM2M/AwaLWM2M/compare/0.1.2...0.1.3)

**Implemented enhancements:**

- Add Process ID of each daemon to log. [\#23](https://github.com/FlowM2M/AwaLWM2M/pull/23) ([DavidAntliff](https://github.com/DavidAntliff))
- Updated contributing guide [\#22](https://github.com/FlowM2M/AwaLWM2M/pull/22) ([cdewbery](https://github.com/cdewbery))
- Make log output more consistent between daemon applications. [\#19](https://github.com/FlowM2M/AwaLWM2M/pull/19) ([DavidAntliff](https://github.com/DavidAntliff))
- Static API Initialisation code and tests [\#18](https://github.com/FlowM2M/AwaLWM2M/pull/18) ([seank-img](https://github.com/seank-img))
- Add server application tutorial to documentation [\#17](https://github.com/FlowM2M/AwaLWM2M/pull/17) ([cdewbery](https://github.com/cdewbery))
- Bootstrap test [\#16](https://github.com/FlowM2M/AwaLWM2M/pull/16) ([DavidAntliff](https://github.com/DavidAntliff))
- Added documentation for example app, pinned 3rd party components [\#15](https://github.com/FlowM2M/AwaLWM2M/pull/15) ([cdewbery](https://github.com/cdewbery))
- documentation update [\#13](https://github.com/FlowM2M/AwaLWM2M/pull/13) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Rework factory bootstrap [\#12](https://github.com/FlowM2M/AwaLWM2M/pull/12) ([cdewbery](https://github.com/cdewbery))
- Initial add of static API and test framework to test static API [\#10](https://github.com/FlowM2M/AwaLWM2M/pull/10) ([seank-img](https://github.com/seank-img))
- Updated tool tests and fixed return code when reading from a write-only resource [\#9](https://github.com/FlowM2M/AwaLWM2M/pull/9) ([rolandbewick](https://github.com/rolandbewick))

**Fixed bugs:**

- Bootstrap test [\#16](https://github.com/FlowM2M/AwaLWM2M/pull/16) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix permissions on bash scripts [\#14](https://github.com/FlowM2M/AwaLWM2M/pull/14) ([cdewbery](https://github.com/cdewbery))
- Fix memory leak. [\#11](https://github.com/FlowM2M/AwaLWM2M/pull/11) ([DavidAntliff](https://github.com/DavidAntliff))

**Merged pull requests:**

- update to user guide, contribute and example files [\#25](https://github.com/FlowM2M/AwaLWM2M/pull/25) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))
- Updated hyper-links in README [\#24](https://github.com/FlowM2M/AwaLWM2M/pull/24) ([cdewbery](https://github.com/cdewbery))
- Fix memory leak with client endpoint name in bootstrap server. [\#21](https://github.com/FlowM2M/AwaLWM2M/pull/21) ([DavidAntliff](https://github.com/DavidAntliff))
- Example.md updated - text and images added. [\#20](https://github.com/FlowM2M/AwaLWM2M/pull/20) ([TonyWalsworthImg](https://github.com/TonyWalsworthImg))

## [0.1.2](https://github.com/FlowM2M/AwaLWM2M/tree/0.1.2) (2016-02-26)
**Implemented enhancements:**

- Fixed spacing before brackets in ServerReadResponse\_GetValueAs functions [\#5](https://github.com/FlowM2M/AwaLWM2M/pull/5) ([rolandbewick](https://github.com/rolandbewick))
- Add addressFamily option to awa\_clientd [\#4](https://github.com/FlowM2M/AwaLWM2M/pull/4) ([cdewbery](https://github.com/cdewbery))
- Changed how Responses store opaque and object link values, allowing t… [\#3](https://github.com/FlowM2M/AwaLWM2M/pull/3) ([rolandbewick](https://github.com/rolandbewick))

**Fixed bugs:**

- coap: Fix coap abstration for contiki [\#8](https://github.com/FlowM2M/AwaLWM2M/pull/8) ([seank-img](https://github.com/seank-img))
- Fixed memory leak when freeing Execute payload. [\#7](https://github.com/FlowM2M/AwaLWM2M/pull/7) ([rolandbewick](https://github.com/rolandbewick))
- coap: Change libcoap abstraction to use separate AddressType [\#6](https://github.com/FlowM2M/AwaLWM2M/pull/6) ([seank-img](https://github.com/seank-img))
- Fix CI scripts. [\#2](https://github.com/FlowM2M/AwaLWM2M/pull/2) ([DavidAntliff](https://github.com/DavidAntliff))
- Fix memory leak. [\#1](https://github.com/FlowM2M/AwaLWM2M/pull/1) ([DavidAntliff](https://github.com/DavidAntliff))



\* *This Change Log was automatically generated by [github_changelog_generator](https://github.com/skywinder/Github-Changelog-Generator)*
