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

// Spitfire headers
#include <spitfire/spitfire.h>

#include <spitfire/util/cConsoleApplication.h>
#include <spitfire/util/cString.h>

#include <spitfire/platform/pipe.h>

#include <spitfire/storage/file.h>
#include <spitfire/storage/filesystem.h>

typedef spitfire::string_t string_t;
typedef spitfire::ostringstream_t ostringstream_t;

class cTarget
{
public:
  string_t sName;
};

class cProject
{
public:
  static bool DependenciesCompare(const cProject* pLeft, const cProject* pRight);

  string_t sName;
  string_t sURL;
  string_t sFolderName;

  std::vector<cTarget*> targets;

private:
  bool IsDependentOn(const cProject& rhs) const;

  std::vector<cProject*> dependencies;
};

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

inline bool cProject::DependenciesCompare(const cProject* pLeft, const cProject* pRight)
{
  assert(pLeft != nullptr);
  assert(pRight != nullptr);
  return pLeft->IsDependentOn(*pRight);
}

class cBuildManager : public spitfire::cConsoleApplication
{
public:
  cBuildManager(int argc, const char* const* argv);

  bool IsError() const { return bIsError; }
  string_t GetError() const { return sErrorMessage; }

private:
  virtual void _PrintHelp() const;
  virtual string_t _GetVersion() const;
  virtual bool _Run(); // NOTE: This may not be run at all, for example if "--help" is the first argument

  void SetError(const string_t& sErrorMessage);

  string_t GetXMLFilePath() const;

  void LoadFromXMLFile();
  void ListAllProjects();
  void BuildAllProjects();

  void Clone(const cProject& project);
  void Build(const cProject& project);
  void Test(const cProject& project);

  void Process(const cProject& project);

  std::vector<cProject*> projects;

  string_t sWorkingFolder;

  bool bIsError;
  string_t sErrorMessage;
};

cBuildManager::cBuildManager(int argc, const char* const* argv) :
  spitfire::cConsoleApplication(argc, argv),
  bIsError(false)
{
}

void cBuildManager::SetError(const string_t& _sErrorMessage)
{
  bIsError = true;
  sErrorMessage = _sErrorMessage;
}

string_t cBuildManager::GetXMLFilePath() const
{
  return TEXT("~/.") + GetApplicationName() + TEXT("/build.xml");
}

void cBuildManager::LoadFromXMLFile()
{
  const string_t sXMLFilePath = GetXMLFilePath();
  if (!spitfire::filesystem::FileExists(sXMLFilePath)) {
    SetError(TEXT("XML File \"") + sXMLFilePath + TEXT("\" doesn't exist"));
    return;
  }
}

void cBuildManager::Clone(const cProject& project)
{
  string_t sPossiblyGitProtocol = project.sURL.substr(0, 3);
  bool bIsGit = (sPossiblyGitProtocol == TEXT("git"));

  string_t sCommand;
  if (bIsGit) sCommand = TEXT("git clone clone --depth 1");
  else sCommand = TEXT("svn co");

  sCommand += TEXT(" ") + project.sURL + TEXT(" ") + sWorkingFolder + project.sFolderName;

  int iReturnCode = -1;
  std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
  if (iReturnCode != 0) {
    ostringstream_t o;
    o<<TEXT("cBuildManager::Clone Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
    SetError(o.str());
  } else {
    #ifdef BUILD_DEBUG
    std::wcout<<TEXT("cBuildManager::Clone Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
    #endif
  }
}

void cBuildManager::Build(const cProject& project)
{
  string_t sCommand;

  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    const cTarget* pTarget = project.targets[i];
    const cTarget& target = *pTarget;

    // Run cmake
    sCommand = TEXT("cmake ") + spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sName);

    {
      int iReturnCode = -1;
      std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
      if (iReturnCode != 0) {
        ostringstream_t o;
        o<<TEXT("cBuildManager::Build cmake process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        SetError(o.str());
      } else {
        #ifdef BUILD_DEBUG
        std::wcout<<TEXT("cBuildManager::Build cmake process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        #endif
      }
    }

    // Run cmake
    sCommand = TEXT("make ") + spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sName);

    {
      int iReturnCode = -1;
      std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
      if (iReturnCode != 0) {
        ostringstream_t o;
        o<<TEXT("cBuildManager::Build make process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        SetError(o.str());
      } else {
        #ifdef BUILD_DEBUG
        std::wcout<<TEXT("cBuildManager::Build make process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        #endif
      }
    }
  }
}

void cBuildManager::Test(const cProject& project)
{
  /*
  string_t sCommand;

  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    const cTarget* pTarget = project.targets[i];
    const cTarget& target = *pTarget;

    // Run application
    sCommand = spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sName, target.sName);

    {
      int iReturnCode = -1;
      std::string sBuffer = spitfire::platform::PipeReadToString(sCommand, iReturnCode);
      if (iReturnCode != 0) {
        ostringstream_t o;
        o<<TEXT("cBuildManager::Test Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        SetError(o.str());
      } else {
        #ifdef BUILD_DEBUG
        std::wcout<<TEXT("cBuildManager::Test Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
        #endif
      }
    }
  }*/
}

void cBuildManager::Process(const cProject& project)
{
   Clone(project);
   if (bIsError) return;

   Build(project);
   if (bIsError) return;

   Test(project);
   if (bIsError) return;
}

void cBuildManager::ListAllProjects()
{
  LoadFromXMLFile();
  if (IsError()) return;

  if (projects.empty()) {
    SetError(TEXT("No projects found"));
    return;
  }

  const size_t n = projects.size();
  for (size_t i = 0; i < n; i++) {
    const cProject* pProject = projects[i];
    ASSERT(pProject != nullptr);
    std::cout<<spitfire::string::ToUTF8(pProject->sName)<<std::endl;

    const size_t nTargets = pProject->targets.size();
    for (size_t j = 0; j < nTargets; j++) {
      const cTarget* pTarget = pProject->targets[i];
      ASSERT(pTarget != nullptr);
      std::cout<<spitfire::string::ToUTF8(pTarget->sName)<<std::endl;
    }
  }
}

void cBuildManager::BuildAllProjects()
{
  LoadFromXMLFile();
  if (IsError()) return;

  if (projects.empty()) {
    SetError(TEXT("No projects found"));
    return;
  }

  std::list<cProject*> projectsSorted;

  // Create a list of our projects that is sorted by least dependencies to most dependencies
  {
    const size_t n = projects.size();
    for (size_t i = 0; i < n; i++) {
      projectsSorted.push_back(projects[i]);
    }

    // TODO: Actually call sort
    //std::sort(projectsSorted.begin(), projectsSorted.end(), cProject::DependenciesCompare);
  }

  spitfire::filesystem::cScopedTemporaryFolder temp;

  sWorkingFolder = temp.GetFolder();

  std::list<cProject*>::const_iterator iter = projectsSorted.begin();
  const std::list<cProject*>::const_iterator iterEnd = projectsSorted.end();
  while (iter != iterEnd) {
    const cProject& project = *(*iter);
    Process(project);
    if (bIsError) break;

    iter++;
  };
}

void cBuildManager::_PrintHelp() const
{
  const string_t sXMLFilePath = GetXMLFilePath();

  std::cout<<"Usage: "<<spitfire::string::ToUTF8(GetApplicationName())<<" [OPTION]"<<std::endl;
  std::cout<<std::endl;
  std::cout<<"  -b, -build, --build  build a list of projects specified in "<<spitfire::string::ToUTF8(sXMLFilePath)<<std::endl;
  std::cout<<"  -l, -list, --list    list the projects specified in "<<spitfire::string::ToUTF8(sXMLFilePath)<<std::endl;
  std::cout<<std::endl;
  std::cout<<"  -help, --help        display this help and exit"<<std::endl;
  std::cout<<"  -version, --version  output version information and exit"<<std::endl;
}

string_t cBuildManager::_GetVersion() const
{
  ostringstream_t o;
  o<<GetApplicationName();
  o<<" 0.1";
  return o.str();
}

bool cBuildManager::_Run()
{
  const size_t n = GetArgumentCount();
  if (n != 1) {
    SetError(TEXT("Invalid arguments"));
  } else {
    const string_t& sArgument = GetArgument(0);
    if ((sArgument == TEXT("-b")) || (sArgument == TEXT("-build")) || (sArgument == TEXT("--build"))) BuildAllProjects();
    else if ((sArgument == TEXT("-l")) || (sArgument == TEXT("-list")) || (sArgument == TEXT("--list"))) ListAllProjects();
    else SetError(TEXT("Unknown argument"));
  }

  if (IsError()) {
    string_t sError = GetError();
    std::cerr<<spitfire::string::ToUTF8(sError)<<std::endl;
    return false;
  }

  return true;
}

/*

<project name="Library">
  <clone name="Library" url="git://breathe.git.sourceforge.net/gitroot/breathe/breathe" folder="library"/>
</project>

<project name="Shared">
  <clone url="git://breathe.git.sourceforge.net/gitroot/breathe/breathe" folder="shared"/>
</project>

<project name="OpenSkate">
   <dependency name="Library"/>
   <dependency name="Shared"/>
   <clone name="Skate" url="https://openskate.svn.sourceforge.net/svnroot/openskate" folder="openskate"/>
   <target name="OpenSkate" application="skate" folder="project"/>
</project>

<project name="Test">
   <dependency name="Library"/>
   <clone name="Test" url="https://openskate.svn.sourceforge.net/svnroot/openskate" folder="test"/>
   <target name="Test FBO" application="test_fbo" folder="test_fbo"/>
   <target name="Test Font" application="test_font" folder="test_font"/>
   <target name="Test Permutations" application="test_permutations" folder="test_permutations"/>
</project>

<project name="Crank">
   <dependency name="Library"/>
   <dependency name="Shared"/>
   <clone name="Crank" url="https://openskate.svn.sourceforge.net/svnroot/openskate" folder="crank"/>
   <target name="Crank" application="crank" folder="project"/>
</project>

<project name="Drive">
   <dependency name="Library"/>
   <dependency name="Shared"/>
   <clone name="Drive" url="https://drivecity.svn.sourceforge.net/svnroot/drivecity/drive" folder="drive"/>
   <target name="Drive" application="drive" folder="project"/>
</project>

<project name="Sudoku">
   <dependency name="Library"/>
   <dependency name="Shared"/>
   <clone name="Sudoku" url="http://sudokubang.svn.sourceforge.net/svnroot/sudokubang/sudoku" folder="sudoku"/>
   <target name="Sudoku" application="sudoku" folder="project"/>
</project>


*/



/*


class cReportProjectResult
{
public:
  cReportProjectResult();

  bool IsPass() const { return bIsPass; }
  bool IsFail() const { return bIsFail; }

  const string_t& GetName() const { return sName; }
  const string_t& GetContent() const;

private:
  bool bIsPass;
  string_t sName;
  string_t sContent;
};

cReportProjectResult::cReportProjectResult() :
  bIsPass(true)
{
}

const string_t& cReportProjectResult::GetContent() const
{
  // Only failed results have content
  ASSERT(IsFail());
  return sContent;
}

class cReportProject
{
public:
  explicit cReportProject(const string_t& sProjectName);

  bool IsSuccess() const { return bIsSuccess; }

  const string_t& GetName() const { return sProjectName; }
  const std::vector<cReportProjectResult*>& GetResults() const { return results; }

  void AddResult(const string_t& sTestName, bool bIsPass, const string_t& sContent = TEXT(""));

private:
  string_t sProjectName;
  bool bIsSuccess;
  std::vector<cReportProjectResult*> results;
};

cReportProject::cReportProject(const string_t& _sProjectName) :
  sProjectName(_sProjectName),
  bIsSuccess(true)
{
}

void cReportProject::AddResult(const string_t& sTestName, bool bIsPass, const string_t& sContent)
{
  if (!bIsPass) bIsSuccess = false;

  cReportProjectResult* pResult = new cReportProjectResult(sTestName, sContent, bIsPass);
  results.push_back(pResult);
}

class cReport
{
public:
  cReport();

  bool IsSuccess() const { return bIsSuccess; }

  const std::vector<cReportProject*>& GetProjects() const { return projects; }

  void AddPass(const string_t& sProjectName, const string_t& sTestName);
  void AddFail(const string_t& sProjectName, const string_t& sTestName, const string& sContent);

private:
  cReportProject* GetProject(const string_t& sProjectName);

  bool bIsSuccess;
  std::vector<cReportProject*> projects;
};

cReport::cReport() :
  bIsSuccess(true)
{
}

void cReport::GetProject(const string_t& sProjectName)
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

  ASSERT(pProject != nullptr);
  return pProject;
}

void cReport::AddPass(const string_t& sProjectName, const string_t& sTestName)
{
  cReportProject* pProject = GetProject(sProjectName);

  ASSERT(pProject != nullptr);
  pProject->AddResult(sTestName, true);
}

void cReport::AddFail(const string_t& sProjectName, const string_t& sTestName, const string& sContent)
{
  if (!bIsPass) bIsSuccess = false;

  cReportProject* pProject = GetProject(sProjectName);

  ASSERT(pProject != nullptr);
  pProject->AddResult(sTestName, false, sContent);
}


enum class STATE
{
  NOT_CHECKED_OUT_NOT_BUILT,
  CHECKED_OUT_NOT_BUILT,
  CHECKED_OUT_BUILT,

  CHECKOUT_FAILED,
  BUILD_FAILED
};

class cProject
{
public:
  cProject();

  const std::vector<string_t>& GetRequiredProjects() const { return requires; }
  void AddRequiredProject(const string_t& sProject) { requires.push_back(sProject); }

  const string_t& GetURL() const { return sURL; }
  void SetURL(const string_t& _sURL) { sURL = _sURL; }

  bool IsCheckedOut() const { return (state == CHECKED_OUT_NOT_BUILT) || (state == CHECKED_OUT_BUILT); }
  bool IsBuilt() const { return (state == CHECKED_OUT_BUILT); }
  void SetState(STATE _state) { state = _state; }

private:
  std::vector<string_t> requires;
  string_t sURL;
  STATE state;
};

cProject::cProject() :
  state(STATE::NOT_CHECKED_OUT_NOT_BUILT)
{
}

class cManager
{
public:
  void LoadProjectsList();
  bool CheckoutAndBuildAllProjects();

  bool IsError() const { return !errors.empty(); }
  void SetError(const string_t& sProjectName, const string_t& sError) { errors.push_back(sProjectName + ": " + sError); }

private:
  std::vector<string_t> errors;
  std::map<string_t, cProject*> projects;
};

void cManager::LoadProjectsList()
{
  string_t sName = ;
  cProject* pProject = new cProject;
  pProject->SetURL(sURL);
  for each requirement {
    pProject->AddRequiredProject(sRequiredProject);
  }
}

bool cManager::CheckoutAndBuildAllProjects()
{
  for (each project) {
    pProject->SetState(STATE::NOT_CHECKED_OUT_NOT_BUILT);
  }

  for (each project) {
    const string_t sName = ;
    const string_t sURL = pProject->GetURL();
    std::vector<string_t> vParts = string::Split(sURL, TEXT("://"));
    if (!vParts.empty()) {
      string_t sCommand;

      const string_t sProtocol = vParts[0];

      if (!pProject->IsCheckedOut()) {
        // Ok, we want to check out this project
        if (sProtocol == TEXT("git")) {
          // Assume git ("git://breathe.git.sourceforge.net/gitroot/breathe/breathe")
          // git clone --depth 1 git://breathe.git.sourceforge.net/gitroot/breathe/breathe breathe

          sCommand = TEXT("git clone --depth 1 ") + sURL + TEXT(" ") + sName;
        } else {
          // Assume svn ("http://sudokubang.svn.sourceforge.net/svnroot/sudokubang")
          //svn co http://sudokubang.svn.sourceforge.net/svnroot/sudokubang sudoku

          sCommand = TEXT("svn co ") + sURL + TEXT(" ") + sName;
        }
      } else {
        // Ok, we have checked out this project before, we just want to update
        ChangeDirectory(sName);

        if (sProtocol == TEXT("git")) {
          sCommand = TEXT("git pull");
        } else {
          sCommand = TEXT("svn update");
        }
      }

      ExecuteCommand(sCommand);

      // We have now either checked out or updated, we can now build
      if (DirectoryExists(TEXT("project"))) {
        // Move into the directory that contains the cmake stuff
        ChangeDirectory(TEXT("project"));

        ExecuteCommand(TEXT("cmake ."));

        PipeExecuteCommand(TEXT("make"));
        if (pipe returned 1) {
          sLastLinesOfOutput = get last 15 lines of sOutput;
          for (each line of sLastLinesOfOutput) {
            SetError(sName, sLastLinesOfOutput);
          }
        }
      }
    }
  }

  const size_t n = errors.size();
  for (size_t i = 0; i < n; i++) std::cout<<errors[i]<<std::endl;

  return errors.empty();
}

void Run()
{
  cReport report;

  // Perform checking out, building and unit testing
  {
    cManager manager;

    manager.LoadProjectsList(report);

    manager.CheckoutAndBuildAllProjects(report);
  }


  // Create the xml tree for the report
  {
    xml::document document;

    const std::vector<cReportProject*>& projects = report.GetProjects();
    const size_t nProjects = projects.size();
    for (size_t iProject = 0; iProject < nProjects; iProject++) {
      const std::vector<cReportProjectResult*>& results = project[iProject].GetResults();
      const size_t nResults = results.size();
      for (size_t iResult = 0; iResult < nResults; iResult++) {
        const cReportProjectResult& result = results[iResult];
        document.AddNode();
        node.SetAttribute(TEXT("name"), result.GetName());
        if (result.IsPass()) node.SetAttribute(TEXT("status"), TEXT("passed"));
        else {
          node.SetAttribute(TEXT("status"), TEXT("failed"));
          node.SetContent(results.GetContent());
        }
      }
    }

    // Write the xml file
    xml::writer writer;

    writer.WriteToFile(document);
  }

  // Post xml file to http://chris.iluo.net/buildall
  {
    ...
  }
}


*/

int main(int argc, char** argv)
{
  int iReturnValue = EXIT_FAILURE;

  {
    cBuildManager manager(argc, argv);

    iReturnValue = manager.Run();
  }

  return iReturnValue;
}

