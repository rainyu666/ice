macro(installGitHooks)
    set(gitHooks "${CMAKE_SOURCE_DIR}/.git/hooks")

    if(EXISTS ${gitHooks}) # git client-side hooks are not available on CI
        # additionally check for (and automatically install) the git hooks we want
        set(gitPrepareCommitMsgHook "${CMAKE_SOURCE_DIR}/.git/hooks/prepare-commit-msg")
        if(NOT EXISTS ${gitPrepareCommitMsgHook})
            message("* Installing git-hook teufel-prepare-commit-msg")
            file(CREATE_LINK ../../teufel-prepare-commit-msg ${gitPrepareCommitMsgHook} SYMBOLIC)
        endif()
        set(gitPrePushHook "${CMAKE_SOURCE_DIR}/.git/hooks/pre-push")
        file(REMOVE ${gitPrePushHook})
        if(NOT EXISTS ${gitPrePushHook})
            message("* Installing git-hook teufel-pre-push")
            file(CREATE_LINK ../../teufel-pre-push ${gitPrePushHook} SYMBOLIC)
        endif()
    endif()
endmacro()
