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
#include <spitfire/storage/xml.h>

typedef spitfire::string_t string_t;
typedef spitfire::ostringstream_t ostringstream_t;

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
  void Build(const cProject& project, const cTarget& target);
  void Test(const cProject& project, const cTarget& target);

  void Build(const cProject& project);
  void Test(const cProject& project);

  void Process(const cProject& project);

  std::vector<cProject> projects;

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
  return spitfire::filesystem::MakeFilePath(spitfire::filesystem::GetHomeConfigurationFilesDirectory(), GetApplicationName(), TEXT("build.xml"));
}

/*
<build>
  <project name="Library" url="git://breathe.git.sourceforge.net/gitroot/breathe/breathe" folder="library">
  </project>

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

  <project name="Tetris" url="https://firestartergame.svn.sourceforge.net/svnroot/firestartergame/tetris" folder="tetris">
    <dependency name="Library"/>
    <dependency name="Shared"/>
    <target name="Tetris" application="tetris" folder="project"/>
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

  <project name="Test" url="git://breathe.git.sourceforge.net/gitroot/breathe/test" folder="test">
    <dependency name="Library"/>
    <target name="Test libopengmm Fade In" application="openglmm_fadein" folder="openglmm_fadein"/>
    <target name="Test libopengmm FBO" application="openglmm_fbo" folder="openglmm_fbo"/>
    <target name="Test libopengmm Font" application="openglmm_font" folder="openglmm_font"/>
    <target name="Test libopengmm Gears" application="openglmm_gears" folder="openglmm_gears"/>
    <target name="Test libopengmm Geometry" application="openglmm_geometry" folder="openglmm_geometry"/>
    <target name="Test libopengmm Heightmap" application="openglmm_heightmap" folder="openglmm_heightmap"/>
    <target name="Test libopengmm Permutations" application="permutations" folder="permutations"/>
    <target name="Test libopengmm Size" application="size_test" folder="size_test"/>
    <target name="Source Cleaner" application="source_cleaner" folder="source_cleaner"/>
    <target name="Test xdgmm" application="xdgmm" folder="xdgmm"/>
  </project>
</build>
*/

void cBuildManager::LoadFromXMLFile()
{
  projects.clear();

  const string_t sXMLFilePath = GetXMLFilePath();
  std::cout<<"cBuildManager::LoadFromXMLFile \""<<spitfire::string::ToUTF8(sXMLFilePath)<<"\""<<std::endl;
  if (!spitfire::filesystem::FileExists(sXMLFilePath)) {
    SetError(TEXT("XML File \"") + sXMLFilePath + TEXT("\" doesn't exist"));
    return;
  }

  // Read the xml file
  spitfire::xml::document document;

  {
    spitfire::xml::reader reader;

    reader.ReadFromFile(document, sXMLFilePath);
  }

  // Parse the xml file
  spitfire::xml::cNode::iterator iterProject(document);
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

    for (spitfire::xml::cNode::iterator iter = iterProject.GetFirstChild(); iter.IsValid(); iter.Next()) {
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

        if (!iter.GetAttribute("folder", target.sFolder)) {
          SetError(TEXT("build.xml project contains a target without a folder"));
          return;
        }

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

void cBuildManager::Clone(const cProject& project)
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
  } else {
    #ifdef BUILD_DEBUG
    std::wcout<<TEXT("cBuildManager::Clone Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
    #endif
  }
}

void cBuildManager::Build(const cProject& project, const cTarget& target)
{
  string_t sCommand;

  // Run cmake
  sCommand = TEXT("cmake ") + spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder) + TEXT("/CMakeLists.txt");

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

  // Run make
  sCommand = TEXT("make ") + spitfire::filesystem::MakeFilePath(sWorkingFolder, project.sFolderName, target.sFolder);

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

void cBuildManager::Build(const cProject& project)
{
  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    Build(project, project.targets[i]);
    if (IsError()) return;
  }
}

void cBuildManager::Test(const cProject& project, const cTarget& target)
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
      o<<TEXT("cBuildManager::Test Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      SetError(o.str());
    } else {
      #ifdef BUILD_DEBUG
      std::wcout<<TEXT("cBuildManager::Test Process \"")<<sCommand<<TEXT("\" returned ")<<iReturnCode<<TEXT(", sBuffer=\"")<<spitfire::string::ToString_t(sBuffer)<<TEXT("\"")<<std::endl;
      #endif
    }
  }
}

void cBuildManager::Test(const cProject& project)
{
  const size_t n = project.targets.size();
  for (size_t i = 0; i < n; i++) {
    Test(project, project.targets[i]);
    if (IsError()) return;
  }
}

void cBuildManager::Process(const cProject& project)
{
   Clone(project);
   if (bIsError) return;

   Build(project);
   if (bIsError) return;

   //Test(project);
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
    const cProject& project = projects[i];
    std::cout<<spitfire::string::ToUTF8(project.sName)<<std::endl;

    const size_t nTargets = project.targets.size();
    for (size_t j = 0; j < nTargets; j++) {
      const cTarget& target = project.targets[i];
      std::cout<<spitfire::string::ToUTF8(target.sName)<<std::endl;
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

  // Create a list of our projects that is sorted by least dependencies to most dependencies
  std::vector<cProject> projectsSorted = projects;
  std::sort(projectsSorted.begin(), projectsSorted.end(), cProject::DependenciesCompare);

  spitfire::filesystem::cScopedTemporaryFolder temp;

  sWorkingFolder = temp.GetFolder();

  const size_t n = projects.size();
  for (size_t i = 0; i < n; i++) {
    const cProject& project = projects[i];
    Process(project);
    if (bIsError) break;
  };

  assert(!IsError());
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

