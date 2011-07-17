// Standard headers
#include <cassert>
#include <cmath>
#include <cstring>

#include <string>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <map>
#include <vector>
#include <list>

// Boost headers
#include <boost/asio.hpp>

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/util/cConsoleApplication.h>
#include <spitfire/util/cString.h>

#include <spitfire/platform/pipe.h>

#include <spitfire/storage/file.h>
#include <spitfire/storage/filesystem.h>
#include <spitfire/storage/json.h>
#include <spitfire/storage/xml.h>

#include <spitfire/communication/http.h>
#include <spitfire/communication/network.h>

typedef spitfire::string_t string_t;
typedef spitfire::ostringstream_t ostringstream_t;


class cReportResult
{
public:
  cReportResult();

  const string_t& GetName() const { return sName; }
  void SetName(const string_t& _sName) { sName = _sName; }

  bool IsNotRun() const { return (state == STATE::NOT_RUN); }
  bool IsPassed() const { return (state == STATE::PASSED); }
  bool IsFailed() const { return (state == STATE::FAILED); }

  void SetNotRun() { state = STATE::NOT_RUN; }
  void SetPassed() { state = STATE::PASSED; }
  void SetFailed() { state = STATE::FAILED; }

private:
  string_t sName;
  enum class STATE {
    NOT_RUN,
    PASSED,
    FAILED
  };
  STATE state;
};

cReportResult::cReportResult() :
  state(STATE::NOT_RUN)
{
}


class cReportTarget
{
public:
  bool IsSuccess() const;

  const string_t& GetName() const { return sName; }
  void SetName(const string_t& _sName) { sName = _sName; }
  const std::vector<cReportResult*>& GetResults() const { return results; }

  void SetTestResultNotRun(const string_t& sTestName);
  void SetTestResultPassed(const string_t& sTestName);
  void SetTestResultFailed(const string_t& sTestName);

private:
  cReportResult* GetOrCreateTest(const string_t& sTestName);

  string_t sName;
  std::vector<cReportResult*> results;
};

cReportResult* cReportTarget::GetOrCreateTest(const string_t& sTestName)
{
  //std::cout<<"cReportTarget::GetOrCreateTest \""<<spitfire::string::ToUTF8(sTestName)<<"\""<<std::endl;
  cReportResult* pResult = nullptr;

  // Find the test if it has already been added
  const size_t n = results.size();
  for (size_t i = 0; i < n; i++) {
    //std::cout<<"cReportTarget::GetOrCreateTest "<<i<<" is "<<spitfire::string::ToUTF8(results[i]->GetName())<<std::endl;
    if (results[i]->GetName() == sTestName) {
      //std::cout<<"cReportTarget::GetOrCreateTest Found"<<std::endl;
      pResult = results[i];
      break;
    }
  }

  // We didn't find the test so we need to create a new one
  if (pResult == nullptr) {
    //std::cout<<"cReportTarget::GetOrCreateTest Not found, creating"<<std::endl;
    pResult = new cReportResult;
    pResult->SetName(sTestName);
    pResult->SetNotRun();
    results.push_back(pResult);
  }

  return pResult;
}

void cReportTarget::SetTestResultNotRun(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetNotRun();
}

void cReportTarget::SetTestResultPassed(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetPassed();
}

void cReportTarget::SetTestResultFailed(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetFailed();
}




class cReportProject
{
public:
  explicit cReportProject(const string_t& sName);

  bool IsSuccess() const;

  // Project
  const string_t& GetName() const { return sName; }
  const std::vector<cReportResult*>& GetResults() const { return results; }
  void SetTestResultNotRun(const string_t& sTestName);
  void SetTestResultPassed(const string_t& sTestName);
  void SetTestResultFailed(const string_t& sTestName);

  // Target
  const std::vector<cReportTarget*>& GetTargets() const { return targets; }
  void SetTestResultNotRun(const string_t& sTarget, const string_t& sTestName);
  void SetTestResultPassed(const string_t& sTarget, const string_t& sTestName);
  void SetTestResultFailed(const string_t& sTarget, const string_t& sTestName);

private:
  cReportResult* GetOrCreateTest(const string_t& sTestName);

  cReportTarget* GetOrCreateTarget(const string_t& sName);

  string_t sName;
  std::vector<cReportResult*> results;
  std::vector<cReportTarget*> targets;
};

cReportProject::cReportProject(const string_t& _sName) :
  sName(_sName)
{
}

cReportResult* cReportProject::GetOrCreateTest(const string_t& sTestName)
{
  //std::cout<<"cReportProject::GetOrCreateTest \""<<spitfire::string::ToUTF8(sTestName)<<"\""<<std::endl;
  cReportResult* pResult = nullptr;

  // Find the test if it has already been added
  const size_t n = results.size();
  for (size_t i = 0; i < n; i++) {
    //std::cout<<"cReportProject::GetOrCreateTest "<<i<<" is "<<spitfire::string::ToUTF8(results[i]->GetName())<<std::endl;
    if (results[i]->GetName() == sTestName) {
      //std::cout<<"cReportProject::GetOrCreateTest Found"<<std::endl;
      pResult = results[i];
      break;
    }
  }

  // We didn't find the test so we need to create a new one
  if (pResult == nullptr) {
    //std::cout<<"cReportProject::GetOrCreateTest Not found, creating"<<std::endl;
    pResult = new cReportResult;
    pResult->SetName(sTestName);
    pResult->SetNotRun();
    results.push_back(pResult);
  }

  return pResult;
}

cReportTarget* cReportProject::GetOrCreateTarget(const string_t& sName)
{
  //std::cout<<"cReportProject::GetOrCreateTarget \""<<spitfire::string::ToUTF8(sName)<<"\""<<std::endl;
  cReportTarget* pTarget = nullptr;

  // Find the test if it has already been added
  const size_t n = targets.size();
  for (size_t i = 0; i < n; i++) {
    //std::cout<<"cReportProject::GetOrCreateTarget "<<i<<" is "<<spitfire::string::ToUTF8(targets[i]->GetName())<<std::endl;
    if (targets[i]->GetName() == sName) {
      //std::cout<<"cReportProject::GetOrCreateTarget Found"<<std::endl;
      pTarget = targets[i];
      break;
    }
  }

  // We didn't find the test so we need to create a new one
  if (pTarget == nullptr) {
    //std::cout<<"cReportProject::GetOrCreateTarget Not found, creating"<<std::endl;
    pTarget = new cReportTarget;
    pTarget->SetName(sName);
    targets.push_back(pTarget);
  }

  return pTarget;
}

void cReportProject::SetTestResultNotRun(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetNotRun();
}

void cReportProject::SetTestResultPassed(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetPassed();
}

void cReportProject::SetTestResultFailed(const string_t& sTestName)
{
  cReportResult* pResult = GetOrCreateTest(sTestName);
  assert(pResult != nullptr);
  pResult->SetFailed();
}

void cReportProject::SetTestResultNotRun(const string_t& sTarget, const string_t& sTestName)
{
  cReportTarget* pTarget = GetOrCreateTarget(sTarget);
  assert(pTarget != nullptr);
  pTarget->SetTestResultNotRun(sTestName);
}

void cReportProject::SetTestResultPassed(const string_t& sTarget, const string_t& sTestName)
{
  cReportTarget* pTarget = GetOrCreateTarget(sTarget);
  assert(pTarget != nullptr);
  pTarget->SetTestResultPassed(sTestName);
}

void cReportProject::SetTestResultFailed(const string_t& sTarget, const string_t& sTestName)
{
  cReportTarget* pTarget = GetOrCreateTarget(sTarget);
  assert(pTarget != nullptr);
  pTarget->SetTestResultFailed(sTestName);
}



class cReport
{
public:
  bool IsSuccess() const;

  const std::vector<cReportProject*>& GetProjects() const { return projects; }
  void AddProject(const string_t& sProjectName);

  void AddTest(const string_t& sProjectName, const string_t& sTestName);
  void SetTestResultNotRun(const string_t& sProjectName, const string_t& sTestName);
  void SetTestResultPassed(const string_t& sProjectName, const string_t& sTestName);
  void SetTestResultFailed(const string_t& sProjectName, const string_t& sTestName);

  void AddTest(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName);
  void SetTestResultNotRun(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName);
  void SetTestResultPassed(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName);
  void SetTestResultFailed(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName);

private:
  cReportProject* GetOrCreateProject(const string_t& sProjectName);

  std::vector<cReportProject*> projects;
};

cReportProject* cReport::GetOrCreateProject(const string_t& sProjectName)
{
  cReportProject* pProject = nullptr;

  const size_t n = projects.size();
  for (size_t i = 0; i < n; i++) {
    if (projects[i]->GetName() == sProjectName) {
      pProject = projects[i];
      break;
    }
  }

  // If we still haven't found the project then we need to create one
  if (pProject == nullptr) {
    pProject = new cReportProject(sProjectName);
    projects.push_back(pProject);
  }

  assert(pProject != nullptr);
  return pProject;
}

void cReport::AddProject(const string_t& sProjectName)
{
  GetOrCreateProject(sProjectName);
}

void cReport::AddTest(const string_t& sProjectName, const string_t& sTestName)
{
  SetTestResultNotRun(sProjectName, sTestName);
}

void cReport::SetTestResultNotRun(const string_t& sProjectName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);

  ASSERT(pProject != nullptr);
  pProject->SetTestResultNotRun(sTestName);
}

void cReport::SetTestResultPassed(const string_t& sProjectName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);

  ASSERT(pProject != nullptr);
  pProject->SetTestResultPassed(sTestName);
}

void cReport::SetTestResultFailed(const string_t& sProjectName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);

  ASSERT(pProject != nullptr);
  pProject->SetTestResultFailed(sTestName);
}

void cReport::AddTest(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName)
{
  SetTestResultNotRun(sProjectName, sTargetName, sTestName);
}

void cReport::SetTestResultNotRun(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);
  assert(pProject != nullptr);
  pProject->SetTestResultNotRun(sTargetName, sTestName);
}

void cReport::SetTestResultPassed(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);
  assert(pProject != nullptr);
  pProject->SetTestResultPassed(sTargetName, sTestName);
}

void cReport::SetTestResultFailed(const string_t& sProjectName, const string_t& sTargetName, const string_t& sTestName)
{
  cReportProject* pProject = GetOrCreateProject(sProjectName);
  assert(pProject != nullptr);
  pProject->SetTestResultFailed(sTargetName, sTestName);
}


class cTarget
{
public:
  string_t sName;
  string_t sApplication;
  string_t sFolder;
};

class cProject
{
public:
  static bool DependenciesCompare(const cProject& lhs, const cProject& rhs);

  string_t sName;
  string_t sURL;
  string_t sFolderName;

  std::vector<string_t> dependenciesAsString;
  void BuildDepencenciesGraph(std::vector<cProject>& allProjects); // Fill out dependencies from dependenciesAsString

  std::vector<cTarget> targets;

private:
  bool IsDependentOn(const cProject& rhs) const;

  std::vector<cProject*> dependencies;
};

void cProject::BuildDepencenciesGraph(std::vector<cProject>& allProjects)
{
  const size_t n = dependenciesAsString.size();
  for (size_t i = 0; i < n; i++) {
    const string_t sNameToFind = dependenciesAsString[i];
    bool bFound = false;

    const size_t nProjects = allProjects.size();
    for (size_t j = 0; j < nProjects; j++) {
      if (sNameToFind == allProjects[j].sName) {
        cProject* pProject = &(allProjects[j]);
        dependencies.push_back(pProject);
        bFound = true;
      }
    }

    if (!bFound) {
      std::cerr<<"Dependency \""<<spitfire::string::ToUTF8(sNameToFind)<<"\" not found for project \""<<spitfire::string::ToUTF8(sName)<<"\""<<std::endl;
    }
  }
}

bool cProject::IsDependentOn(const cProject& rhs) const
{
  const size_t n = dependencies.size();
  for (size_t i = 0; i < n; i++) {
    if (dependencies[i]->sName == rhs.sName) return true;

    if (dependencies[i]->IsDependentOn(rhs)) return true;
  }

  return false;
}

// *** Comparison for sorting particles based on depth

inline bool cProject::DependenciesCompare(const cProject& lhs, const cProject& rhs)
{
  return lhs.IsDependentOn(rhs);
}

class cBuildManager
{
public:
  explicit cBuildManager(const string_t& sXMLFilePath);

  bool IsError() const { return bIsError; }
  string_t GetError() const { return sErrorMessage; }

  void ListAllProjects(cReport& report);
  void BuildAllProjects(cReport& report);

private:
  void SetError(const string_t& sErrorMessage);

  void LoadFromXMLFile();

  // Targets
  bool IsJavaTarget(const cProject& project, const cTarget& target) const;
  void BuildJava(cReport& report, const cProject& project, const cTarget& target);
  void BuildCPlusPlus(cReport& report, const cProject& project, const cTarget& target);
  void TestJava(cReport& report, const cProject& project, const cTarget& target);
  void TestCPlusPlus(cReport& report, const cProject& project, const cTarget& target);
  void Build(cReport& report, const cProject& project, const cTarget& target);
  void Test(cReport& report, const cProject& project, const cTarget& target);

  // Projects
  void Clone(cReport& report, const cProject& project);
  void Build(cReport& report, const cProject& project);
  void Test(cReport& report, const cProject& project);

  string_t sXMLFilePath;

  std::vector<cProject> projects;

  string_t sWorkingFolder;

  bool bIsError;
  string_t sErrorMessage;
};

cBuildManager::cBuildManager(const string_t& _sXMLFilePath) :
  sXMLFilePath(_sXMLFilePath),
  bIsError(false)
{
}

void cBuildManager::SetError(const string_t& _sErrorMessage)
{
  bIsError = true;
  sErrorMessage = _sErrorMessage;
}

/*
<build>
  <project name="PostCodes" url="git://github.com/pilkch/postcodes.git" folder="postcodes">
    <target name="PostCodes" application="postcodes"/>
  </project>

  <project name="Allocator" url="git://github.com/pilkch/allocator.git" folder="allocator">
    <target name="Allocator" application="allocator"/>
  </project>

  <project name="Library" url="git://github.com/pilkch/library.git" folder="library">
  </project>

  <project name="Buildall" url="git://github.com/pilkch/buildall.git" folder="buildall">
    <dependency name="Library"/>
    <target name="Builall" application="buildall"/>
  </project>

  <project name="Tetris" url="git://github.com/pilkch/tetris.git" folder="tetris">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="Tetris" application="tetris" folder="project"/>
  </project>

  <project name="Test" url="git://github.com/pilkch/test.git" folder="test">
    <dependency name="Library"/>
    <target name="Test libopengmm Fade In" application="openglmm_fadein" folder="openglmm_fadein"/>
    <target name="Test libopengmm FBO" application="openglmm_fbo" folder="openglmm_fbo"/>
    <target name="Test libopengmm Font" application="openglmm_font" folder="openglmm_font"/>
    <target name="Test libopengmm Gears" application="openglmm_gears" folder="openglmm_gears"/>
    <target name="Test libopengmm Geometry" application="openglmm_geometry" folder="openglmm_geometry"/>
    <target name="Test libopengmm Heightmap" application="openglmm_heightmap" folder="openglmm_heightmap"/>
    <target name="Test Permutations" application="permutations" folder="permutations"/>
    <target name="Test Size" application="size_test" folder="size_test"/>
    <target name="Source Cleaner" application="source_cleaner" folder="source_cleaner"/>
    <target name="Test xdgmm" application="xdgmm" folder="xdgmm"/>
  </project>
</build>
*/

/*
// Not ready yet

  <project name="Shared" url="https://firestartergame.svn.sourceforge.net/svnroot/firestartergame/shared" folder="shared">
  </project>

  <project name="OpenSkate" url="https://openskate.svn.sourceforge.net/svnroot/openskate/skate" folder="openskate">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="OpenSkate" application="skate" folder="project"/>
  </project>

  <project name="Crank" url="https://firestartergame.svn.sourceforge.net/svnroot/firestartergame/crank" folder="crank">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="Crank" application="crank" folder="project"/>
  </project>

  <project name="Drive" url="https://drivecity.svn.sourceforge.net/svnroot/drivecity/drive" folder="drive">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="Drive" application="drive" folder="project"/>
  </project>

  <project name="Sudoku" url="git://sudokubang.git.sourceforge.net/gitroot/sudokubang/sudokubang" folder="sudoku">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="Sudoku" application="sudoku" folder="project"/>
  </project>

  <project name="FireStarter" url="https://firestartergame.svn.sourceforge.net/svnroot/firestartergame/firestarter" folder="firestarter">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="FireStarter" application="firestarter" folder="project"/>
  </project>
*/

void cBuildManager::LoadFromXMLFile()
{
  projects.clear();

  std::cout<<"cBuildManager::LoadFromXMLFile \""<<spitfire::string::ToUTF8(sXMLFilePath)<<"\""<<std::endl;
  if (!spitfire::filesystem::FileExists(sXMLFilePath)) {
    SetError(TEXT("XML File \"") + sXMLFilePath + TEXT("\" doesn't exist"));
    return;
  }

  spitfire::document::cDocument document;

  {
    // Read the xml file
    spitfire::xml::reader reader;

    reader.ReadFromFile(document, sXMLFilePath);
  }

  // Parse the xml file
  spitfire::document::cNode::iterator iterProject(document);
  if (!iterProject.IsValid()) {
    SetError(TEXT("build.xml does not contain valid xml data"));
    return;
  }

  iterProject.FindChild("build");
  if (!iterProject.IsValid()) {
    SetError(TEXT("build.xml does not contain a build root node"));
    return;
  }

  iterProject.FindChild("project");
  while (iterProject.IsValid()) {
    cProject project;
    if (!iterProject.GetAttribute("name", project.sName)) {
      SetError(TEXT("build.xml contains a project without a name"));
      return;
    }

    std::cout<<"project \""<<spitfire::string::ToUTF8(project.sName)<<"\""<<std::endl;

    if (!iterProject.GetAttribute("url", project.sURL)) {
      SetError(TEXT("build.xml contains a project without a url"));
      return;
    }

    std::cout<<"url \""<<spitfire::string::ToUTF8(project.sURL)<<"\""<<std::endl;

    if (!iterProject.GetAttribute("folder", project.sFolderName)) {
      SetError(TEXT("build.xml contains a project without a folder"));
      return;
    }

    std::cout<<"folder \""<<spitfire::string::ToUTF8(project.sFolderName)<<"\""<<std::endl;

    for (spitfire::document::cNode::iterator iter = iterProject.GetFirstChild(); iter.IsValid(); iter.Next()) {
      const std::string sType = iter.GetName();

      if (sType == "dependency") {
        //<dependency name="Library"/>
        std::cout<<"dependency"<<std::endl;
        string_t sDependency;
        if (!iter.GetAttribute("name", sDependency)) {
          SetError(TEXT("build.xml contains a dependency without a name"));
          return;
        }

        project.dependenciesAsString.push_back(sDependency);
      } else if (sType == "target") {
        //<target name="OpenSkate" application="skate" folder="project"/>
        std::cout<<"target"<<std::endl;

        cTarget target;

        if (!iter.GetAttribute("name", target.sName)) {
          SetError(TEXT("build.xml project contains a target without a name"));
          return;
        }

        if (!iter.GetAttribute("application", target.sApplication)) {
          SetError(TEXT("build.xml project contains a target without a application"));
          return;
        }

        iter.GetAttribute("folder", target.sFolder);

        project.targets.push_back(target);
      } else {
        std::cerr<<"build.xml contains a project (\""<<spitfire::string::ToUTF8(project.sName)<<"\") with an unknown type \""<<spitfire::string::ToUTF8(sType)<<"\""<<std::endl;
      }
    }

    projects.push_back(project);

    iterProject.Next("project");
  }

  const size_t n = projects.size();
  for (size_t i = 0; i < n; i++) projects[i].BuildDepencenciesGraph(projects);
}

void cBuildManager::Clone(cReport& report, const cProject& project)
{
  string_t sPossiblyGitProtocol = project.sURL.substr(0, 3);
  bool bIsGit = (sPossiblyGitProtocol == TEXT("git"));

  string_t sCommand;
  if (bIsGit) sCommand = TEXT("git clone --depth 1");
  else sCommand = TEXT("svn co");

  sCommand += TEXT(" ") + project.sURL + TEXT(" ") + spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName);

  std::wcout<<TEXT("cBuildManager::Clone sCommand=\"")<<sCommand<<TEXT("\"")<<std::endl;

  int iReturnCode = -1;
  std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
  if (iReturnCode != 0) {
    ostringstream_t o;
    o<<TEXT("cBuildManager::Clone Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
    SetError(o.str());
    report.SetTestResultFailed(project.sName, TEXT("clone"));
  } else {
    #ifdef BUILD_DEBUG
    std::wcout<<TEXT("cBuildManager::Clone Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
    #endif
    report.SetTestResultPassed(project.sName, TEXT("clone"));
  }
}

bool cBuildManager::IsJavaTarget(const cProject& project, const cTarget& target) const
{
  const string_t sTargetFolder = spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder);

  // If there is a build.xml file within this directory then it is probably an Ant make file and we should treat it as a Java project
  const string_t sBuildXML = spitfire::filesystem::MakeFilePath(sTargetFolder, TEXT("build.xml"));
  return spitfire::filesystem::FileExists(sBuildXML);
}

void cBuildManager::BuildJava(cReport& report, const cProject& project, const cTarget& target)
{
  // Run ant build
  {
    const string_t sCommand = TEXT("ant build");

    int iReturnCode = -1;
    std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
    if (iReturnCode != 0) {
      ostringstream_t o;
      o<<TEXT("cBuildManager::BuildJava ant build process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
      report.SetTestResultFailed(project.sName, target.sName, TEXT("ant build"));
      return;
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::BuildJava ant build process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
      report.SetTestResultPassed(project.sName, target.sName, TEXT("ant build"));
    }
  }
}

void cBuildManager::BuildCPlusPlus(cReport& report, const cProject& project, const cTarget& target)
{
  // Run cmake
  {
    const string_t sCommand = TEXT("cmake .");

    int iReturnCode = -1;
    std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
    if (iReturnCode != 0) {
      ostringstream_t o;
      o<<TEXT("cBuildManager::BuildCPlusPlus cmake process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
      report.SetTestResultFailed(project.sName, target.sName, TEXT("cmake"));
      return;
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::BuildCPlusPlus cmake process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
      report.SetTestResultPassed(project.sName, target.sName, TEXT("cmake"));
    }
  }

  // Run make
  {
    const string_t sCommand = TEXT("make");

    int iReturnCode = -1;
    std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
    if (iReturnCode != 0) {
      ostringstream_t o;
      o<<TEXT("cBuildManager::BuildCPlusPlus make process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
      report.SetTestResultFailed(project.sName, target.sName, TEXT("make"));
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::BuildCPlusPlus make process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
      report.SetTestResultPassed(project.sName, target.sName, TEXT("make"));
    }
  }
}

void cBuildManager::Build(cReport& report, const cProject& project, const cTarget& target)
{
  const string_t sTargetFolder = spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder);

  // Change to the target directory so that cmake and make will work
  spitfire::filesystem::cScopedDirectoryChange changeDirectory(sTargetFolder);

  if (IsJavaTarget(project, target)) BuildJava(report, project, target);
  else BuildCPlusPlus(report, project, target);
}

void cBuildManager::Build(cReport& report, const cProject& project)
{
  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    Build(report, project, project.targets[i]);
  }
}

void cBuildManager::TestJava(cReport& report, const cProject& project, const cTarget& target)
{
  // TODO: Pass --unittest and actually test the built application
  /*// Run ant Application
  {
    const string_t sCommand = TEXT("ant Application");

    int iReturnCode = -1;
    std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
    if (iReturnCode != 0) {
      ostringstream_t o;
      o<<TEXT("cBuildManager::TestJava ant Application process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
      report.SetTestResultFailed(project.sName, TEXT("ant Application"));
      return;
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::TestJava ant Application process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
      report.SetTestResultPassed(project.sName, target.sName, TEXT("ant Application"));
    }
  }*/
}

void cBuildManager::TestCPlusPlus(cReport& report, const cProject& project, const cTarget& target)
{
  const string_t sApplication = spitfire::filesystem::MakeFilePath(spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder), target.sApplication);

  // Make sure that the application has been built sucessfully
  assert(spitfire::filesystem::FileExists(sApplication));

  // Run application with unittest parameter
  const string_t sCommand = sApplication + TEXT(" --unittest");

  {
    int iReturnCode = -1;
    std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
    if (iReturnCode != 0) {
      ostringstream_t o;
      o<<TEXT("cBuildManager::TestCPlusPlus Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::TestCPlusPlus Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
    }
  }
}

void cBuildManager::Test(cReport& report, const cProject& project, const cTarget& target)
{
  const string_t sTargetFolder = spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder);

  if (IsJavaTarget(project, target)) TestJava(report, project, target);
  else TestCPlusPlus(report, project, target);
}

void cBuildManager::Test(cReport& report, const cProject& project)
{
  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    Test(report, project, project.targets[i]);
    if (IsError()) return;
  }
}

void cBuildManager::ListAllProjects(cReport& report)
{
  LoadFromXMLFile();
  if (IsError()) return;

  if (projects.empty()) {
    SetError(TEXT("No projects found"));
    return;
  }

  const size_t n = projects.size();
  for (size_t i = 0; i < n; i++) {
    const cProject& project = projects[i];
    std::cout<<spitfire::string::ToUTF8(project.sName)<<std::endl;

    const size_t nTargets = project.targets.size();
    for (size_t j = 0; j < nTargets; j++) {
      const cTarget& target = project.targets[i];
      std::cout<<spitfire::string::ToUTF8(target.sName)<<std::endl;
    }
  }
}

void cBuildManager::BuildAllProjects(cReport& report)
{
  LoadFromXMLFile();
  if (IsError()) return;

  if (projects.empty()) {
    SetError(TEXT("No projects found"));
    return;
  }

  // Create a list of our projects that is sorted by least dependencies to most dependencies
  std::vector<cProject> projectsSorted = projects;
  std::sort(projectsSorted.begin(), projectsSorted.end(), cProject::DependenciesCompare);

  // Add an entry for each project to the report
  const size_t nProjects = projects.size();
  for (size_t i = 0; i < nProjects; i++) {
    cProject& project = projects[i];
    report.AddProject(project.sName);
    report.AddTest(project.sName, TEXT("clone"));
  }

  spitfire::filesystem::cScopedTemporaryFolder temp;

  sWorkingFolder = temp.GetFolder();

  // Pull all projects
  std::wcout<<TEXT("Cloning Projects")<<std::endl;
  for (size_t i = 0; i < nProjects; i++) {
    const cProject& project = projects[i];
    Clone(report, project);
  }

  // Add an entry for each project to the report
  for (size_t i = 0; i < nProjects; i++) {
    cProject& project = projects[i];

    const size_t nTargets = project.targets.size();
    for (size_t iTarget = 0; iTarget < nTargets; iTarget++) {
      cTarget& target = project.targets[iTarget];
      if (IsJavaTarget(project, target)) {
        report.AddTest(project.sName, target.sName, TEXT("ant build"));
      } else {
        report.AddTest(project.sName, target.sName, TEXT("cmake"));
        report.AddTest(project.sName, target.sName, TEXT("make"));
      }
    }
  }

  // If cloning was successful then we are ready to build and test our projects
  if (!bIsError) {
    std::wcout<<TEXT("Building and Testing Projects")<<std::endl;
    // Compile and test all projects
    for (size_t i = 0; i < nProjects; i++) {
      const cProject& project = projects[i];
      Build(report, project);
      //Test(report, project);
    };
  }
}

class cApplication : public spitfire::cConsoleApplication
{
public:
  cApplication(int argc, const char* const* argv);

  string_t GetConfigXMLFilePath() const;

private:
  virtual void _PrintHelp() const;
  virtual string_t _GetVersion() const;
  virtual bool _Run(); // NOTE: This may not be run at all, for example if "--help" is the first argument

  string_t GetBuildXMLFilePath() const;

  void ListAllProjects();
  void BuildAllProjects();
};

cApplication::cApplication(int argc, const char* const* argv) :
  spitfire::cConsoleApplication(argc, argv)
{
}

string_t cApplication::GetConfigXMLFilePath() const
{
  return spitfire::filesystem::MakeFilePath(spitfire::filesystem::GetHomeConfigurationFilesDirectory(), GetApplicationName(), TEXT("config.xml"));
}

string_t cApplication::GetBuildXMLFilePath() const
{
  return spitfire::filesystem::MakeFilePath(spitfire::filesystem::GetHomeConfigurationFilesDirectory(), GetApplicationName(), TEXT("build.xml"));
}

void cApplication::_PrintHelp() const
{
  const string_t sXMLFilePath = GetBuildXMLFilePath();

  std::cout<<"Usage: "<<spitfire::string::ToUTF8(GetApplicationName())<<" [OPTION]"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"  -b, -build, --build  build a list of projects specified in "<<spitfire::string::ToUTF8(sXMLFilePath)<<std::endl;
  std::cout<<"  -l, -list, --list    list the projects specified in "<<spitfire::string::ToUTF8(sXMLFilePath)<<std::endl;
  std::cout<<std::endl;
  std::cout<<"  -help, --help        display this help and exit"<<std::endl;
  std::cout<<"  -version, --version  output version information and exit"<<std::endl;
}

string_t cApplication::_GetVersion() const
{
  ostringstream_t o;
  o<<GetApplicationName();
  o<<" 0.1";
  return o.str();
}

void cApplication::ListAllProjects()
{
  cReport report;

  {
    cBuildManager manager(GetBuildXMLFilePath());

    manager.ListAllProjects(report);
  }

  // Print the list of projects
  {
    const std::vector<cReportProject*>& projects = report.GetProjects();
    const size_t nProjects = projects.size();
    for (size_t iProject = 0; iProject < nProjects; iProject++) {
      std::cout<<spitfire::string::ToUTF8(projects[iProject]->GetName())<<std::endl;
    }
  }
}

class cConfig
{
public:
  explicit cConfig(const cApplication& application);

  void Load();

  const std::string& GetHostUTF8() const { return sHostUTF8; }
  const std::string& GetPathUTF8() const { return sPathUTF8; }
  const std::string& GetSecretUTF8() const { return sSecretUTF8; }

private:
  void Clear();

  const cApplication& application;

  std::string sHostUTF8;
  std::string sPathUTF8;
  std::string sSecretUTF8;
};

cConfig::cConfig(const cApplication& _application) :
  application(_application)
{
}

void cConfig::Clear()
{
  sHostUTF8.clear();
  sPathUTF8.clear();
  sSecretUTF8.clear();
}

void cConfig::Load()
{
  Clear();

  const string_t sXMLFilePath = application.GetConfigXMLFilePath();
  std::cout<<"cConfig::Load \""<<spitfire::string::ToUTF8(sXMLFilePath)<<"\""<<std::endl;
  if (!spitfire::filesystem::FileExists(sXMLFilePath)) {
    std::wcerr<<TEXT("XML File \"")<<sXMLFilePath<<TEXT("\" doesn't exist")<<std::endl;
    return;
  }

  spitfire::document::cDocument document;

  {
    // Read the xml file
    spitfire::xml::reader reader;

    reader.ReadFromFile(document, sXMLFilePath);
  }

  // Parse the xml file
  spitfire::document::cNode::iterator iterAccount(document);
  if (!iterAccount.IsValid()) {
    std::wcerr<<TEXT("config.xml does not contain valid xml data")<<std::endl;
    return;
  }

  //<config>
  //  <account host="chris.iluo.net" path="/tests/submit.php" secret="secret">
  //</config>

  iterAccount.FindChild("config");
  if (!iterAccount.IsValid()) {
    std::wcerr<<TEXT("config.xml does not contain a config root node")<<std::endl;
    return;
  }

  iterAccount.FindChild("account");
  if (iterAccount.IsValid()) {
    if (!iterAccount.GetAttribute("host", sHostUTF8)) {
      std::wcerr<<TEXT("config.xml contains an account without a host")<<std::endl;
      return;
    }

    if (!iterAccount.GetAttribute("path", sPathUTF8)) {
      std::wcerr<<TEXT("config.xml contains a project without a path")<<std::endl;
      return;
    }

    if (!iterAccount.GetAttribute("secret", sSecretUTF8)) {
      std::wcerr<<TEXT("config.xml contains a project without a secret")<<std::endl;
      return;
    }
  }
}

void cApplication::BuildAllProjects()
{
  cReport report;

  {
    cBuildManager manager(GetBuildXMLFilePath());

    manager.BuildAllProjects(report);
  }


  spitfire::json::cDocument document;

  // Create the json tree for the report
  {
    spitfire::json::cNode* pDocumentNode = &document;
    pDocumentNode->SetTypeObject();

    const std::vector<cReportProject*>& projects = report.GetProjects();
    const size_t nProjects = projects.size();
    for (size_t iProject = 0; iProject < nProjects; iProject++) {
      const cReportProject& project = *projects[iProject];
      spitfire::json::cNode* pProjectNode = document.CreateNode("project");
      pDocumentNode->AppendChild(pProjectNode);
      pProjectNode->SetTypeObject();
      pProjectNode->SetAttribute("name", project.GetName());

      // Project results
      {
        spitfire::json::cNode* pProjectResults = document.CreateNode("results");
        pProjectNode->AppendChild(pProjectResults);
        pProjectResults->SetTypeArray();

        const std::vector<cReportResult*>& results = project.GetResults();
        const size_t nResults = results.size();
        for (size_t iResult = 0; iResult < nResults; iResult++) {
          const cReportResult& result = *results[iResult];
          spitfire::json::cNode* pResultNode = document.CreateNode();
          pProjectResults->AppendChild(pResultNode);
          pResultNode->SetTypeObject();
          assert(!result.GetName().empty());
          pResultNode->SetAttribute("name", result.GetName());
          if (result.IsNotRun()) pResultNode->SetAttribute("status", TEXT("notrun"));
          if (result.IsPassed()) pResultNode->SetAttribute("status", TEXT("passed"));
          else pResultNode->SetAttribute("status", TEXT("failed"));
        }
      }

      // Target results
      {
        spitfire::json::cNode* pTargetResults = document.CreateNode("targets");
        pProjectNode->AppendChild(pTargetResults);
        pTargetResults->SetTypeArray();

        const std::vector<cReportTarget*>& targets = project.GetTargets();
        const size_t nTargets = targets.size();
        for (size_t iTarget = 0; iTarget < nTargets; iTarget++) {
          const cReportTarget& target = *targets[iTarget];
          spitfire::json::cNode* pTargetNode = document.CreateNode();
          pTargetResults->AppendChild(pTargetNode);
          pTargetNode->SetTypeObject();
          pTargetNode->SetAttribute("name", target.GetName());

          spitfire::json::cNode* pResultsNode = document.CreateNode();
          pTargetNode->AppendChild(pResultsNode);
          pResultsNode->SetTypeArray();
          pResultsNode->SetName("results");

          const std::vector<cReportResult*>& results = target.GetResults();
          const size_t nResults = results.size();
          for (size_t iResult = 0; iResult < nResults; iResult++) {
            const cReportResult& result = *results[iResult];
            spitfire::json::cNode* pResultNode = document.CreateNode();
            pResultsNode->AppendChild(pResultNode);
            pResultNode->SetTypeObject();
            assert(!result.GetName().empty());
            pResultNode->SetAttribute("name", result.GetName());
            if (result.IsNotRun()) pResultNode->SetAttribute("status", TEXT("notrun"));
            if (result.IsPassed()) pResultNode->SetAttribute("status", TEXT("passed"));
            else pResultNode->SetAttribute("status", TEXT("failed"));
          }
        }
      }
    }
  }

  const string_t sFilePath = spitfire::filesystem::MakeFilePath(spitfire::filesystem::GetHomeDirectory(), TEXT("results.json"));

  // Write the json file for debugging purposes
  {
    spitfire::json::writer writer;

    writer.WriteToFile(document, sFilePath);
  }

  // Post json file to http://chris.iluo.net/buildall
  {
    // Read host, path and secret from .config/buildall/config.xml
    cConfig config(*this);
    config.Load();

    if (!config.GetHostUTF8().empty() && !config.GetPathUTF8().empty()) {
      spitfire::network::http::cRequest request;
      request.SetMethodPost();
      request.SetHost(spitfire::string::ToString_t(config.GetHostUTF8()));
      request.SetPath(spitfire::string::ToString_t(config.GetPathUTF8()));
      if (!config.GetSecretUTF8().empty()) request.AddValue("secret", config.GetSecretUTF8());
      request.AddPostFileFromPath("file", sFilePath);

      spitfire::network::http::cHTTP http;
      http.SendRequest(request);
      bool bResult = http.IsSuccessful();
      std::cout<<"Result="<<bResult<<std::endl;
    }


  }
}

bool cApplication::_Run()
{
  string_t sError;

  const size_t n = GetArgumentCount();
  if (n != 1) sError = TEXT("Invalid number of arguments");
  else {
    const string_t& sArgument = GetArgument(0);
    if ((sArgument == TEXT("-b")) || (sArgument == TEXT("-build")) || (sArgument == TEXT("--build"))) BuildAllProjects();
    else if ((sArgument == TEXT("-l")) || (sArgument == TEXT("-list")) || (sArgument == TEXT("--list"))) ListAllProjects();
    else sError = TEXT("Unknown argument \"") + sArgument + TEXT("\"");
  }

  if (!sError.empty()) {
    std::cerr<<spitfire::string::ToUTF8(sError)<<std::endl;
    return false;
  }

  return true;
}

int main(int argc, char** argv)
{
  int iReturnValue = EXIT_FAILURE;

  {
    cApplication application(argc, argv);

    iReturnValue = application.Run();
  }

  return iReturnValue;
}

