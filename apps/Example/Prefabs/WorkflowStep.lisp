(StackView Name="WorkflowStep" class="card stack-card workflow-step"
  (TextBlock Name="WorkflowStepNumber" class="workflow-number" Text=(bind "DataContext/Title")
    ForegroundColor=(bind "DataContext/PrimaryColor"))
  (TextBlock Name="WorkflowStepTitle" class="workflow-title" Text=(bind "DataContext/Subtitle"))
  (TextBlock Name="WorkflowStepBody" class="card-body" Text=(bind "DataContext/Body")))
